project('discord', 'c', 'cpp', version: '1.0.0')
warning_level = 3
cpp_std = 'c++23'
debug = true
optimization = 3
b_lto = true
b_pgo = 'use'

cpp = meson.get_compiler('cpp')

add_project_arguments(
	cpp.get_supported_arguments(
		'-D_WIN32_WINNT=_WIN32_WINNT_WIN10',
		'-std=c++23',
		'-masm=intel',
		'-O3',
	),
	language: 'cpp',
)

add_project_link_arguments(
	cpp.get_supported_arguments(
		'-static',
		'-s',
		'-std=c++23',
		'-ld3dcompiler',
	),
	language: 'cpp',
)

minhook = subproject('minhook')
libpng = subproject('libpng', default_options: ['default_library=static'])
curl = subproject('curl', default_options: ['default_library=static'])
discord = subproject('discord')

library(
	'discord',
	include_directories: [
		'src',
	],
	cpp_pch : 'src/helpers.h',
	dependencies : [
		minhook.get_variable('minhook_dep'),
		libpng.get_variable('libpng_dep'),
		curl.get_variable('curl_dep'),
		discord.get_variable('discord_dep'),
	],
	sources : [
		'src/mod.cpp',
		'src/diva.cpp',
	],
	name_prefix: ''
)
