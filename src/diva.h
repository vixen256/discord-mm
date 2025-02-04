#pragma once
namespace diva {
struct Vec2 {
	f32 x;
	f32 y;

	Vec2 () {
		this->x = 0;
		this->y = 0;
	}

	Vec2 (f32 x, f32 y) {
		this->x = x;
		this->y = y;
	}

	Vec2 operator+ (Vec2 other) { return Vec2 (this->x + other.x, this->y + other.y); }
	Vec2 operator- (Vec2 other) { return Vec2 (this->x - other.x, this->y - other.y); }
	Vec2 operator* (Vec2 other) { return Vec2 (this->x * other.x, this->y * other.y); }
	Vec2 operator/ (Vec2 other) { return Vec2 (this->x / other.x, this->y / other.y); }
	Vec2 operator+ (f32 offset) { return Vec2 (this->x + offset, this->y + offset); }
	Vec2 operator* (f32 scale) { return Vec2 (this->x * scale, this->y * scale); }
	Vec2 operator/ (f32 scale) { return Vec2 (this->x / scale, this->y / scale); }
};

struct Vec3 {
	f32 x;
	f32 y;
	f32 z;

	Vec3 () {
		this->x = 0;
		this->y = 0;
		this->z = 0;
	}

	Vec3 (f32 x, f32 y, f32 z) {
		this->x = x;
		this->y = y;
		this->z = z;
	}

	Vec3 operator+ (Vec3 other) { return Vec3 (this->x + other.x, this->y + other.y, this->z + other.z); }
	Vec3 operator- (Vec3 other) { return Vec3 (this->x - other.x, this->y - other.y, this->z - other.z); }
	Vec3 operator* (Vec3 other) { return Vec3 (this->x * other.x, this->y * other.y, this->z * other.z); }
	Vec3 operator/ (Vec3 other) { return Vec3 (this->x / other.x, this->y / other.y, this->z / other.z); }
	Vec3 operator* (f32 scale) { return Vec3 (this->x * scale, this->y * scale, this->z * scale); }
	Vec3 operator/ (f32 scale) { return Vec3 (this->x / scale, this->y / scale, this->z / scale); }
};

struct Vec4 {
	f32 x;
	f32 y;
	f32 z;
	f32 w;

	Vec4 () {
		this->x = 0;
		this->y = 0;
		this->z = 0;
		this->w = 0;
	}

	Vec4 (f32 x, f32 y, f32 z, f32 w) {
		this->x = x;
		this->y = y;
		this->z = z;
		this->w = w;
	}

	bool contains (Vec2 location) { return location.x > this->x && location.x < this->z && location.y > this->y && location.y < this->w; }
	Vec4 operator* (f32 scale) { return Vec4 (this->x * scale, this->y * scale, this->z * scale, this->w * scale); }
};

struct mat4 {
	Vec4 x;
	Vec4 y;
	Vec4 z;
	Vec4 w;
};

FUNCTION_PTR_H (void *, operatorNew, u64);
FUNCTION_PTR_H (void *, operatorDelete, void *);
struct string;
FUNCTION_PTR_H (void, FreeString, string *);
template <typename T>
T *
allocate (u64 count) {
	return (T *)(operatorNew (count * sizeof (T)));
}

inline void
deallocate (void *p) {
	operatorDelete (p);
}

#pragma pack(push, 8)
struct string {
	union {
		char data[16];
		char *ptr;
	};
	u64 length;
	u64 capacity;

	char *c_str () {
		if (this->capacity > 15) return this->ptr;
		else return this->data;
	}

	string () {
		memset (this->data, 0, sizeof (this->data));
		this->length   = 0;
		this->capacity = 0x0F;
	}
	string (const char *cstr) {
		u64 len = strlen (cstr) + 1;
		if (len > 15) {
			u64 new_len = len | 0xF;
			this->ptr   = allocate<char> (new_len);
			strcpy (this->ptr, cstr);
			this->length   = len - 1;
			this->capacity = new_len;
		} else {
			strcpy (this->data, cstr);
			this->length   = len - 1;
			this->capacity = 15;
		}
	}
	string (char *cstr) {
		u64 len = strlen (cstr) + 1;
		if (len > 15) {
			u64 new_len = len | 0xF;
			this->ptr   = allocate<char> (new_len);
			strcpy (this->ptr, cstr);
			this->length   = len - 1;
			this->capacity = new_len;
		} else {
			strcpy (this->data, cstr);
			this->length   = len - 1;
			this->capacity = 15;
		}
	}

	~string () { FreeString (this); }

	bool operator== (string &rhs) {
		if (!this->c_str () || !rhs.c_str ()) return false;
		return strcmp (this->c_str (), rhs.c_str ()) == 0;
	}
	bool operator== (char *rhs) {
		if (!this->c_str () || !rhs) return false;
		return strcmp (this->c_str (), rhs) == 0;
	}
	auto operator<=> (string &rhs) {
		if (!this->c_str () || !rhs.c_str ()) return 1;
		return strcmp (this->c_str (), rhs.c_str ());
	}
	auto operator<=> (char *rhs) {
		if (!this->c_str () || !rhs) return 1;
		return strcmp (this->c_str (), rhs);
	}

	void extend (size_t len) {
		size_t extension = this->length + 1 + len;
		if (extension <= this->capacity) return;

		size_t new_capacity = extension | 0x0F;
		auto old            = this->c_str ();
		auto new_ptr        = allocate<char> (new_capacity);
		strcpy (new_ptr, old);

		if (this->capacity > 15) deallocate (this->ptr);
		this->ptr      = new_ptr;
		this->capacity = new_capacity;
	}

	void operator+= (string &rhs) {
		this->extend (rhs.length);
		strcpy (&this->c_str ()[this->length], rhs.c_str ());
		this->length += rhs.length;
		this->c_str ()[this->length] = 0;
	}
	void operator+= (char *rhs) {
		this->extend (strlen (rhs));
		strcpy (&this->c_str ()[this->length], rhs);
		this->length += strlen (rhs);
		this->c_str ()[this->length] = 0;
	}
	void operator+= (const char *rhs) {
		this->extend (strlen (rhs));
		strcpy (&this->c_str ()[this->length], rhs);
		this->length += strlen (rhs);
		this->c_str ()[this->length] = 0;
	}
	void operator+= (char rhs) {
		this->extend (1);
		this->c_str ()[this->length] = rhs;
		this->length += 1;
		this->c_str ()[this->length] = 0;
	}
};

template <typename T>
struct listElement {
	listElement<T> *next;
	listElement<T> *previous;
	T current;

	bool operator== (const listElement<T> &rhs) { return this->current == rhs->current; }
	bool operator== (const T &rhs) { return this->current == rhs; }
};

template <typename T>
struct list {
	listElement<T> *root;
	u64 length;

	listElement<T> *begin () { return this->length ? this->root->next : this->root; }
	listElement<T> *end () { return this->root; }
};

template <typename K, typename V>
struct pair {
	K key;
	V value;
};

template <typename K, typename V>
struct mapElement {
	mapElement<K, V> *left;
	mapElement<K, V> *parent;
	mapElement<K, V> *right;
	bool isBlack;
	bool isNull;

	pair<K, V> pair;

	mapElement<K, V> *next () {
		auto ptr = this;
		if (ptr->right->isNull) {
			while (!ptr->parent->isNull && ptr == ptr->parent->right)
				ptr = ptr->parent;
			ptr = ptr->parent;
		} else {
			ptr = ptr->right;
			while (!ptr->left->isNull)
				ptr = ptr->left;
		}
		return ptr;
	}

	K *key () { return &this->pair.key; }

	V *value () { return &this->pair.value; }
};

template <typename K, typename V>
struct map {
	mapElement<K, V> *root;
	u64 length;

	map () {
		this->root          = allocate<mapElement<K, V>> (1);
		this->root->left    = this->root;
		this->root->parent  = this->root;
		this->root->right   = this->root;
		this->root->isBlack = true;
		this->root->isNull  = true;
		this->length        = 0;
	}

	~map () {
		for (auto it = this->begin (); it != this->end (); it = it->next ())
			deallocate (it);
		deallocate (this->root);
	}

	std::optional<V *> find (K key) {
		auto ptr = this->root->parent;
		while (!ptr->isNull)
			if (key == ptr->pair.key) return std::optional (ptr->value ());
			else if (key > ptr->pair.key) ptr = ptr->right;
			else if (key < ptr->pair.key) ptr = ptr->left;
		return std::nullopt;
	}

	mapElement<K, V> *begin () { return this->length ? this->root->left : this->root; }
	mapElement<K, V> *end () { return this->root; }
};

template <typename T>
struct vector {
	T *first;
	T *last;
	void *capacity_end;

	vector () {}
	vector (u64 n) {
		this->first        = allocate<T> (n);
		this->last         = this->first;
		this->capacity_end = (void *)((u64)this->first + (n * sizeof (T)));
	}

	~vector () { deallocate (this->first); }

	std::optional<T *> at (u64 index) {
		if (index >= this->length ()) return std::nullopt;
		return std::optional (&this->first[index]);
	}

	void push_back (T value) {
		if (this->remaining_capcity () > 0) {
			this->first[this->length ()] = value;
			this->last++;
			return;
		}

		u64 new_length = (this->length () + (this->length () / 2)) | 0xF;
		T *new_first   = allocate<T> (new_length);
		u64 old_length = (u64)this->last - (u64)this->first;
		memcpy ((void *)new_first, (void *)this->first, old_length);
		deallocate (this->first);

		this->first        = new_first;
		this->last         = (T *)((u64)new_first + old_length);
		this->capacity_end = (void *)((u64)new_first + (new_length * sizeof (T)));

		this->first[this->length ()] = value;
		this->last++;
	}

	std::optional<T *> find (const std::function<bool (T *)> &func) {
		for (auto it = this->begin (); it != this->end (); it++)
			if (func (it)) return std::optional (it);
		return std::nullopt;
	}

	u64 length () { return ((u64)this->last - (u64)this->first) / sizeof (T); }
	u64 capacity () { return ((u64)this->capacity_end - (u64)this->first) / sizeof (T); }
	u64 remaining_capcity () { return this->capacity () - this->length (); }

	T *begin () { return this->first; }
	T *end () { return this->last; }
};

template <typename T>
struct _stringRangeBase {
	T *data;
	T *end;

	// Technically incorrect but always seems to work
	T *c_str () { return data; }

	_stringRangeBase () {
		data = 0;
		end  = 0;
	}
	_stringRangeBase (const T *str);
	_stringRangeBase (size_t length) {
		if (length <= 0) length = 8;
		data = allocate<T> (length);
		end  = data + length;
		memset (data, 0, length);
	}

	~_stringRangeBase () { deallocate (data); }
};
using stringRange  = _stringRangeBase<char>;
using wstringRange = _stringRangeBase<wchar_t>;

enum resolutionMode : u32 {
	RESOLUTION_MODE_QVGA          = 0x00,
	RESOLUTION_MODE_VGA           = 0x01,
	RESOLUTION_MODE_SVGA          = 0x02,
	RESOLUTION_MODE_XGA           = 0x03,
	RESOLUTION_MODE_SXGA          = 0x04,
	RESOLUTION_MODE_SXGAPlus      = 0x05,
	RESOLUTION_MODE_UXGA          = 0x06,
	RESOLUTION_MODE_WVGA          = 0x07,
	RESOLUTION_MODE_WSVGA         = 0x08,
	RESOLUTION_MODE_WXGA          = 0x09,
	RESOLUTION_MODE_FWXGA         = 0x0A,
	RESOLUTION_MODE_WUXGA         = 0x0B,
	RESOLUTION_MODE_WQXGA         = 0x0C,
	RESOLUTION_MODE_HD            = 0x0D,
	RESOLUTION_MODE_FHD           = 0x0E,
	RESOLUTION_MODE_UHD           = 0x0F,
	RESOLUTION_MODE_3KatUHD       = 0x10,
	RESOLUTION_MODE_3K            = 0x11,
	RESOLUTION_MODE_QHD           = 0x12,
	RESOLUTION_MODE_WQVGA         = 0x13,
	RESOLUTION_MODE_qHD           = 0x14,
	RESOLUTION_MODE_XGAPlus       = 0x15,
	RESOLUTION_MODE_1176x664      = 0x16,
	RESOLUTION_MODE_1200x960      = 0x17,
	RESOLUTION_MODE_WXGA1280x900  = 0x18,
	RESOLUTION_MODE_SXGAMinus     = 0x19,
	RESOLUTION_MODE_FWXGA1366x768 = 0x1A,
	RESOLUTION_MODE_WXGAPlus      = 0x1B,
	RESOLUTION_MODE_HDPlus        = 0x1C,
	RESOLUTION_MODE_WSXGA         = 0x1D,
	RESOLUTION_MODE_WSXGAPlus     = 0x1E,
	RESOLUTION_MODE_1920x1440     = 0x1F,
	RESOLUTION_MODE_QWXGA         = 0x20,
	RESOLUTION_MODE_MAX           = 0x21,
};

template <typename T>
struct PvDbIndexedValue {
	i32 index;
	T value;
};

template <typename T>
struct PvDbIdValue {
	i32 index;
	i32 id;
	T value;
};

struct PvDbPlaceholder {};

struct PvDbExInfo {
	string key;
	string value;
};

struct PvDbField {
	i32 index;
	i32 stageIndex;
	i32 exStageIndex;
};

struct PvDbDifficulty {
	i32 difficulty;
	i32 edition;
	i32 isExtra;
	// Theres something at +0x10 but I cannot figure it out and I don't need that info right now
	i32 unk[5];
	string scriptFile;
	i32 level; // Stars * 2;
	string buttonSoundEffect;
	string successSoundEffect;
	string slideSoundEffect;
	string slideChainStartSoundEffect;
	string slideChainSoundEffect;
	string slideChainSuccessSoundEffect;
	string slideChainFailureSoundEffect;
	string slideTouchSoundEffect;
	vector<PvDbIdValue<string>> motion[6];
	vector<PvDbField> field;
	bool exStage;
	vector<PvDbIndexedValue<string>> items;
	vector<PvDbIdValue<string>> handItems;
	vector<PvDbIndexedValue<string>> editEffects;
	vector<PvDbPlaceholder> unk_240;
	vector<PvDbPlaceholder> unk_258;
	vector<PvDbPlaceholder> unk_270;
	f32 unk_288;
	f32 unk_28C;
	string unk_290;
	string music;
	string illustrator;
	string arranger;
	string manipulator;
	string editor;
	string guitar;
	PvDbExInfo exInfo[4];
	string unk_470;
	vector<PvDbIndexedValue<string>> movies;
	i32 movieSurface;
	bool isMovieOnly;
	string effectSoundEffect;
	vector<string> effectSoundEffectList;
	i32 version;
	i32 scriptFormat;
	i32 highSpeedRate;
	f32 hiddenTiming;
	f32 suddenTiming;
	bool editCharaScale;
	string mixModeScript;
	u64 unk_0x520;
	u64 unk_0x528;

	~PvDbDifficulty () = delete;
};

struct PvDbPerformer {
	i32 type;
	i32 chara;
	i32 costume;
	i32 pv_costume[5];
	bool fixed;
	i32 exclude;
	i32 size;
	i32 pseudo_same_id;
	i32 item[4];
	i32 unk;

	~PvDbPerformer () = delete;
};

struct PvDbExSong {
	i32 chara[6];
	string fileName;
	string name;
	vector<pair<string, string>> auth;
};

struct PvDbAnotherSong {
	string name;
	string fileName;
	string vocalDispName;
	i32 vocalCharaNum;
};

struct PvDbEntry {
	i32 id;
	i32 date;
	string name;
	string nameReading;
	i32 unk_48;
	i32 bpm;
	string soundFile;
	vector<PvDbIndexedValue<string>> lyrics;
	f32 sabiStartTime;
	f32 sabiPlayTime;
	u64 unk_90;
	vector<PvDbPerformer> performers;
	vector<PvDbDifficulty> difficulties[5];
	i32 exSongCharaCount;
	vector<PvDbExSong> exSong;
	bool mdata;
	string mdata_dir;
	vector<PvDbPlaceholder> osage;
	vector<PvDbPlaceholder> stageParam;
	string disp2dSetName;
	i32 targetShadowType;
	i32 titleStart2dField;
	i32 titleEnd2dField;
	i32 titleStart2dLowField;
	i32 titleEnd2dLowField;
	i32 titleStart3dField;
	i32 titleEnd3dField;
	string title2dLayer;
	bool useOsagePlayData;
	string pvExpression;
	vector<PvDbPlaceholder> chreff;
	vector<PvDbPlaceholder> chrcam;
	vector<PvDbPlaceholder> chrmot;
	bool eyesXrotAdjust;
	bool isOldPv;
	i32 eyesAdjust;
	map<i32, PvDbPlaceholder> eyesRotRate;
	vector<PvDbAnotherSong> anotherSong;
	INSERT_PADDING (0x138);
	i32 pack;

	~PvDbEntry () = delete;
};

enum ModuleAttr : i32 {
	Swimsuit     = 1 << 0,
	NoSwap       = 1 << 1,
	FutureSound  = 1 << 2,
	ColorfulTone = 1 << 3,
	Tshirt       = 1 << 4,
};

struct ModuleData {
	i32 id;
	i32 sort_index;
	i32 chara;
	i32 cos;
	INSERT_PADDING (0x5C);
	i32 sprSetId;
	i32 unk_0x70;
	i32 spriteId;
	i32 unk_0x74;
	i32 unk_0x78;
	string name;
	i32 shop_price;
	u64 unk_0xA8;
	u64 unk_0xB0;
	ModuleAttr attr;
};

struct PvSpriteId {
	PvDbEntry *pvData;
	i32 setId;
	i32 bgId[4];
	i32 jkId[4];
	i32 logoId[4];
	i32 thumbId[4];
};

struct PvLoadInfo {
	i32 pvId;
	i32 version;
	i32 difficulty;
	i32 extra;
	i32 level;
	u8 unk_0x14;
	i32 pvId2;
	i32 unk_0x1C;
	u8 unk_0x20;
	u8 unk_0x21;
	i32 unk_0x24;
	u8 unk_0x28;
	i32 modifier;
	i32 modules[6];
	i32 unk_0x48[6];
	i32 accessories[6][5];
	bool accessories_enabled[6][5];
};

struct FontInfo {
	u32 fontId;
	void *rawfont;
	i32 sprId;
	i32 unk_0x14;
	f32 unk_0x18;
	f32 unk_0x1C;
	f32 width;
	f32 height;
	f32 unk_0x28;
	f32 unk_0x2C;
	Vec2 size;
	f32 scaledWidth;
	f32 scaledHeight;
	f32 unk_0x40;
	f32 unk_0x44;

	FontInfo ();
};

struct DrawParams {
	f32 unk_0x00;
	u8 colour[4];
	u8 fillColour[4];
	bool clip;
	Vec4 clipData;
	u32 layer;
	u32 priority;
	resolutionMode resolutionMode;
	u32 unk_0x2C;
	Vec2 textCurrent;
	Vec2 lineOrigin;
	u64 lineLength;
	FontInfo *font;
	u16 unk_0x50;
	u32 unk_0x54;
	u32 unk_0x58;
	u32 unk_0x5C;
	u32 unk_0x60;

	DrawParams ();
};

struct DirectXTexture {
	u64 ref_count;
	u64 unk_0x08;
	ID3D11Texture2D *texture;
	ID3D11Device *device;
	i32 unk_0x30;
	i32 width;
	i32 height;
	i32 unk_0x3C;
};

// Thank you koren
struct SprArgs {
	u32 kind;
	i32 id;
	u8 color[4];
	i32 attr;
	i32 blend;
	i32 index;
	i32 priority;
	i32 layer;
	resolutionMode resolution_mode_screen;
	resolutionMode resolution_mode_sprite;
	Vec3 center;
	Vec3 trans;
	Vec3 scale;
	Vec3 rot;
	Vec2 skew_angle;
	mat4 mat;
	void *texture;
	i32 shader;
	i32 field_AC;
	mat4 transform;
	bool field_F0;
	void *vertex_array;
	size_t num_vertex;
	i32 field_108;
	void *field_110;
	bool set_viewport;
	Vec4 viewport;
	u32 flags;
	Vec2 sprite_size;
	i32 field_138;
	Vec2 texture_pos;
	Vec2 texture_size;
	SprArgs *next;
	DirectXTexture *dx_texture;

	SprArgs ();
};

#pragma pack(pop)

extern vector<PvDbEntry *> *pvs;
extern map<i32, PvSpriteId> *pvSprites;

FUNCTION_PTR_H (SprArgs *, DrawSpr, SprArgs *args);

std::optional<PvDbEntry **> getPvDbEntry (i32 id);
std::optional<PvDbDifficulty *> getPvDbDifficulty (i32 id, i32 difficulty, bool extra);
} // namespace diva
