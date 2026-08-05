# All variables and this file are optional, if they are not present the PG and the
# makefiles will try to parse the correct values from the file system.

meta:
	ADDON_NAME = ofxSteamAudio
	ADDON_DESCRIPTION = Steam Audio 4.8.1 spatial audio for openFrameworks (full C API wrappers)
	ADDON_AUTHOR = ofxSteamAudio contributors
	ADDON_TAGS = "audio" "spatial" "steam" "hrtf" "phonon"
	ADDON_URL = https://github.com/valvesoftware/steam-audio

common:
	ADDON_INCLUDES = libs/steamaudio/include
	ADDON_INCLUDES += src
	# Embree + friends in this package are often x86_64-only; default ray tracer works everywhere.
	ADDON_LIBS_EXCLUDE = libs/steamaudio/lib/%/libembree%
	ADDON_LIBS_EXCLUDE += libs/steamaudio/lib/%/libmath%
	ADDON_LIBS_EXCLUDE += libs/steamaudio/lib/%/libsimd%
	ADDON_LIBS_EXCLUDE += libs/steamaudio/lib/%/libsys%
	ADDON_LIBS_EXCLUDE += libs/steamaudio/lib/%/libtasking%
	ADDON_LIBS_EXCLUDE += libs/steamaudio/lib/%/liblexers%
	ADDON_LIBS_EXCLUDE += libs/steamaudio/lib/%/pkgconfig%
	ADDON_LIBS_EXCLUDE += libs/steamaudio/lib/%/cmake%

osx:
	ADDON_LIBS = libs/steamaudio/lib/osx/libphonon.a
	ADDON_LIBS += libs/steamaudio/lib/osx/libmysofa.a
	ADDON_LIBS += libs/steamaudio/lib/osx/libpffft.a
	ADDON_LDFLAGS = -lz
	ADDON_FRAMEWORKS = Accelerate AudioToolbox CoreAudio

linux64:
	ADDON_LIBS = libs/steamaudio/lib/linux64/libphonon.so

vs:
	ADDON_LIBS = libs/steamaudio/lib/win64/phonon.lib

emscripten:
	ADDON_LIBS = libs/steamaudio/lib/emscripten/WASM/libphonon.a
