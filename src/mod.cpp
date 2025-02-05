#include "diva.h"

using namespace diva;
ID3D11Device *device;
ID3D11DeviceContext *context;
ID3D11ComputeShader *shader;

struct Globals {
	DirectX::XMUINT2 PixelOffset;
	DirectX::XMUINT2 Padding;
};

IDiscordCore *core                  = nullptr;
IDiscordActivityManager *activities = nullptr;

void
UpdateActivityCallback (void *, enum EDiscordResult result) {
	if (result != DiscordResult_Ok) printf ("[discord] Activity update failed: %d\n", result);
}

void
DiscordThread () {
	while (true) {
		if (core != nullptr) core->run_callbacks (core);
		std::this_thread::sleep_for (std::chrono::milliseconds (50));
	}
}

void
CreateDiscord () {
	if (core != nullptr && activities != nullptr) return;
	DiscordCreateParams params = {0};
	DiscordCreateParamsSetDefault (&params);
	params.flags          = DiscordCreateFlags_NoRequireDiscord;
	params.client_id      = 1005179451572752404;
	EDiscordResult result = DiscordCreate (DISCORD_VERSION, &params, &core);
	if (result != DiscordResult_Ok) return;
	activities = core->get_activity_manager (core);

	std::thread t (DiscordThread);
	t.detach ();
}

u64
curlWriteCallback (char *data, u64 size, u64 nmemb, char *buf) {
	strcat (buf, data);
	buf[size * nmemb] = 0;
	return size * nmemb;
}

PvLoadInfo *pvInfo = nullptr;

void
UploadImage () {
	curl_global_init (CURL_GLOBAL_ALL);

	auto curl      = curl_easy_init ();
	auto multipart = curl_mime_init (curl);

	auto part = curl_mime_addpart (multipart);
	curl_mime_name (part, "reqtype");
	curl_mime_data (part, "fileupload", CURL_ZERO_TERMINATED);

	part = curl_mime_addpart (multipart);
	curl_mime_name (part, "time");
	curl_mime_data (part, "1h", CURL_ZERO_TERMINATED);

	part = curl_mime_addpart (multipart);
	curl_mime_name (part, "fileToUpload");
	curl_mime_filedata (part, "tmp.png");

	char buf[256];
	memset (buf, 0, 256);

	curl_easy_setopt (curl, CURLOPT_URL, "https://litterbox.catbox.moe/resources/internals/api.php");
	curl_easy_setopt (curl, CURLOPT_MIMEPOST, multipart);
	curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, curlWriteCallback);
	curl_easy_setopt (curl, CURLOPT_WRITEDATA, buf);

	curl_easy_perform (curl);

	curl_mime_free (multipart);
	curl_easy_cleanup (curl);

	DeleteFile ("tmp.png");

	printf ("[discord] Got image upload: %s\n", buf);

	CreateDiscord ();

	if (activities == nullptr || core == nullptr || pvInfo == nullptr) return;
	if (auto pppv = getPvDbEntry (pvInfo->pvId)) {
		auto pv = **pppv;
		DiscordActivity activity;
		memset (&activity, 0, sizeof (activity));
		strcpy (activity.assets.large_image, buf);
		strcpy (activity.assets.large_text, "Project DIVA MegaMix+");
		strcpy (activity.state, pv->name.c_str ());
		switch (pvInfo->difficulty) {
		case 0:
			strcpy (activity.assets.small_image, "easy");
			strcpy (activity.assets.small_text, "Easy");
			break;
		case 1:
			strcpy (activity.assets.small_image, "normal");
			strcpy (activity.assets.small_text, "Normal");
			break;
		case 2:
			strcpy (activity.assets.small_image, "hard");
			strcpy (activity.assets.small_text, "Hard");
			break;
		case 3:
			strcpy (activity.assets.small_image, "extreme");
			strcpy (activity.assets.small_text, "Extreme");
			break;
		}
		if (pvInfo->extra == true) {
			strcpy (activity.assets.small_image, "extra");
			strcpy (activity.assets.small_text, "ExExtreme");
		}
		activity.timestamps.start = std::chrono::duration_cast<std::chrono::seconds> (std::chrono::system_clock::now ().time_since_epoch ()).count ();

		activities->clear_activity (activities, 0, UpdateActivityCallback);
		activities->update_activity (activities, &activity, 0, UpdateActivityCallback);
	}
}

HOOK (i64, SongEnd, 0x14043B040) {
	CreateDiscord ();
	if (activities == nullptr || core == nullptr) return originalSongEnd ();
	free (pvInfo);
	pvInfo = nullptr;

	DiscordActivity activity;
	memset (&activity, 0, sizeof (activity));
	strcpy (activity.assets.large_image, "miku");
	strcpy (activity.assets.large_text, "Project DIVA MegaMix+");
	strcpy (activity.state, "In menu");
	activities->clear_activity (activities, 0, UpdateActivityCallback);
	activities->update_activity (activities, &activity, 0, UpdateActivityCallback);
	return originalSongEnd ();
}

HOOK (void, SetPvLoadData, 0x14040B600, u64 PvLoadData, PvLoadInfo *info, bool a3) {
	originalSetPvLoadData (PvLoadData, info, a3);
	if (auto sprites = pvSprites->find (info->pvId)) {
		pvInfo = (PvLoadInfo *)malloc (sizeof (PvLoadInfo));
		memcpy (pvInfo, info, sizeof (PvLoadInfo));

		HRESULT hr;
		SprArgs args;
		args.id      = sprites.value ()->jkId[info->difficulty];
		args.layer   = 0;
		auto newArgs = DrawSpr (&args);
		auto texture = newArgs->dx_texture->texture;

		D3D11_TEXTURE2D_DESC desc;
		texture->GetDesc (&desc);

		D3D11_TEXTURE2D_DESC newDesc;
		newDesc.Width              = newArgs->texture_size.x;
		newDesc.Height             = newArgs->texture_size.y;
		newDesc.MipLevels          = 1;
		newDesc.ArraySize          = 1;
		newDesc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;
		newDesc.SampleDesc.Count   = 1;
		newDesc.SampleDesc.Quality = 0;
		newDesc.Usage              = D3D11_USAGE_DEFAULT;
		newDesc.BindFlags          = D3D11_BIND_UNORDERED_ACCESS;
		newDesc.CPUAccessFlags     = 0;
		newDesc.MiscFlags          = 0;

		ID3D11Texture2D *newTexture = nullptr;
		hr                          = device->CreateTexture2D (&newDesc, nullptr, &newTexture);
		if (FAILED (hr)) {
			printf ("CreateTexture2D %lx\n", hr);
			return;
		}

		context->CSSetShader (shader, nullptr, 0);

		Globals data;
		data.PixelOffset.x = newArgs->texture_pos.x;
		data.PixelOffset.y = desc.Height - newArgs->texture_pos.y;

		D3D11_BUFFER_DESC cbDesc;
		cbDesc.ByteWidth           = sizeof (Globals);
		cbDesc.Usage               = D3D11_USAGE_DYNAMIC;
		cbDesc.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
		cbDesc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
		cbDesc.MiscFlags           = 0;
		cbDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA cbData;
		cbData.pSysMem          = &data;
		cbData.SysMemPitch      = 0;
		cbData.SysMemSlicePitch = 0;

		ID3D11Buffer *buffer = nullptr;
		hr                   = device->CreateBuffer (&cbDesc, &cbData, &buffer);
		if (FAILED (hr)) {
			printf ("CreateBuffer %lx\n", hr);
			return;
		}
		context->CSSetConstantBuffers (0, 1, &buffer);

		ID3D11ShaderResourceView *shaderReadTexture = nullptr;
		hr                                          = device->CreateShaderResourceView (texture, nullptr, &shaderReadTexture);
		if (FAILED (hr)) {
			printf ("CreateShaderResourceView %lx\n", hr);
			return;
		}
		context->CSSetShaderResources (0, 1, &shaderReadTexture);

		ID3D11UnorderedAccessView *shaderReadWriteTexture = nullptr;
		hr                                                = device->CreateUnorderedAccessView (newTexture, nullptr, &shaderReadWriteTexture);
		if (FAILED (hr)) {
			printf ("CreateUnorderedAccessView %lx\n", hr);
			return;
		}
		context->CSSetUnorderedAccessViews (0, 1, &shaderReadWriteTexture, nullptr);

		context->Dispatch (newArgs->texture_size.x, newArgs->texture_size.y, 1);

		D3D11_TEXTURE2D_DESC stagingDesc;
		stagingDesc.Width              = newArgs->texture_size.x;
		stagingDesc.Height             = newArgs->texture_size.y;
		stagingDesc.MipLevels          = 1;
		stagingDesc.ArraySize          = 1;
		stagingDesc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;
		stagingDesc.SampleDesc.Count   = 1;
		stagingDesc.SampleDesc.Quality = 0;
		stagingDesc.Usage              = D3D11_USAGE_STAGING;
		stagingDesc.BindFlags          = 0;
		stagingDesc.CPUAccessFlags     = D3D11_CPU_ACCESS_READ;
		stagingDesc.MiscFlags          = 0;

		ID3D11Texture2D *stagingTexture = nullptr;
		hr                              = device->CreateTexture2D (&stagingDesc, nullptr, &stagingTexture);
		if (FAILED (hr)) {
			printf ("CreateTexture2D %lx\n", hr);
			return;
		}

		context->CopyResource (stagingTexture, newTexture);

		D3D11_MAPPED_SUBRESOURCE map;
		hr = context->Map (stagingTexture, 0, D3D11_MAP_READ, 0, &map);
		if (FAILED (hr)) {
			printf ("Map %lx\n", hr);
			return;
		}

		png_image png;
		memset (&png, 0, sizeof (png_image));
		png.version = PNG_IMAGE_VERSION;
		png.width   = newArgs->texture_size.x;
		png.height  = newArgs->texture_size.y;
		png.format  = PNG_FORMAT_RGBA;

		png_image_write_to_file (&png, "tmp.png", 0, map.pData, map.RowPitch, nullptr);

		context->Unmap (stagingTexture, 0);

		std::thread t (UploadImage);
		t.detach ();
	}
}

extern "C" {
__declspec (dllexport) void
init () {
	freopen ("CONOUT$", "w", stdout);

	INSTALL_HOOK (SetPvLoadData);
	INSTALL_HOOK (SongEnd);

	CreateDiscord ();
	if (activities == nullptr || core == nullptr) return;

	DiscordActivity activity;
	memset (&activity, 0, sizeof (activity));
	strcpy (activity.assets.large_image, "miku");
	strcpy (activity.assets.large_text, "Project DIVA MegaMix+");
	strcpy (activity.state, "In menu");

	activities->update_activity (activities, &activity, 0, UpdateActivityCallback);
}

__declspec (dllexport) void
D3DInit (IDXGISwapChain *SwapChain, ID3D11Device *Device, ID3D11DeviceContext *DeviceContext) {
	device  = Device;
	context = DeviceContext;

	ID3DBlob *shaderBlob;
	ID3DBlob *errorBlob;
	D3DCompileFromFile (L"shader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "cs_5_0", 0, 0, &shaderBlob, &errorBlob);
	device->CreateComputeShader (shaderBlob->GetBufferPointer (), shaderBlob->GetBufferSize (), nullptr, &shader);
}
}
