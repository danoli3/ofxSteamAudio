# All variables and this file are optional, if they are not present the PG and the
# makefiles will try to parse the correct values from the file system.
#
# Prebuilt Steam Audio libs: run ./scripts/manage_libs.sh  (download or apothecary build)

meta:
	ADDON_NAME = ofxSteamAudio
	ADDON_DESCRIPTION = Steam Audio spatial audio for openFrameworks (full C API wrappers)
	ADDON_AUTHOR = ofxSteamAudio contributors
	ADDON_TAGS = "audio" "spatial" "steam" "hrtf" "phonon"
	ADDON_URL = https://github.com/ValveSoftware/steam-audio

common:
	ADDON_INCLUDES = libs/steamaudio/include
	ADDON_INCLUDES += src
	# Optional embree extras (x86_64-only in some packages) — use default ray tracer by default
	ADDON_LIBS_EXCLUDE = libs/steamaudio/lib/%/libembree%
	ADDON_LIBS_EXCLUDE += libs/steamaudio/lib/%/libmath%
	ADDON_LIBS_EXCLUDE += libs/steamaudio/lib/%/libsimd%
	ADDON_LIBS_EXCLUDE += libs/steamaudio/lib/%/libsys%
	ADDON_LIBS_EXCLUDE += libs/steamaudio/lib/%/libtasking%
	ADDON_LIBS_EXCLUDE += libs/steamaudio/lib/%/liblexers%
	ADDON_LIBS_EXCLUDE += libs/steamaudio/lib/%/pkgconfig%
	ADDON_LIBS_EXCLUDE += libs/steamaudio/lib/%/cmake%
	ADDON_LIBS_EXCLUDE += libs/steamaudio/lib/%/*.pkl

# ----- Desktop -----
osx:
	ADDON_LIBS = libs/steamaudio/lib/osx/libphonon.a
	ADDON_LIBS += libs/steamaudio/lib/osx/libmysofa.a
	ADDON_LIBS += libs/steamaudio/lib/osx/libpffft.a
	ADDON_LDFLAGS = -lz
	ADDON_FRAMEWORKS = Accelerate AudioToolbox CoreAudio

linux64:
	ADDON_LIBS = libs/steamaudio/lib/linux64/libphonon.a
	ADDON_LIBS += libs/steamaudio/lib/linux64/libmysofa.a
	ADDON_LIBS += libs/steamaudio/lib/linux64/libpffft.a
	ADDON_LDFLAGS = -lz -lpthread -ldl

linuxaarch64:
	ADDON_LIBS = libs/steamaudio/lib/linuxaarch64/libphonon.a
	ADDON_LIBS += libs/steamaudio/lib/linuxaarch64/libmysofa.a
	ADDON_LIBS += libs/steamaudio/lib/linuxaarch64/libpffft.a
	ADDON_LDFLAGS = -lz -lpthread -ldl

linuxarmv7l:
	ADDON_LIBS = libs/steamaudio/lib/linuxarmv7l/libphonon.a
	ADDON_LIBS += libs/steamaudio/lib/linuxarmv7l/libmysofa.a
	ADDON_LIBS += libs/steamaudio/lib/linuxarmv7l/libpffft.a
	ADDON_LDFLAGS = -lz -lpthread -ldl

vs:
	ADDON_LIBS = libs/steamaudio/lib/vs/x64/phonon.lib
	# Optional static deps if present:
	# ADDON_LIBS += libs/steamaudio/lib/vs/x64/mysofa.lib
	# ADDON_LIBS += libs/steamaudio/lib/vs/x64/pffft.lib

msys2:
	ADDON_LIBS = libs/steamaudio/lib/msys2/libphonon.a
	ADDON_LDFLAGS = -lz

# ----- Apple mobile / embedded -----
ios:
	ADDON_LIBS = libs/steamaudio/lib/ios/libphonon.a
	ADDON_LIBS += libs/steamaudio/lib/ios/libmysofa.a
	ADDON_LIBS += libs/steamaudio/lib/ios/libpffft.a
	ADDON_LDFLAGS = -lz
	ADDON_FRAMEWORKS = Accelerate AudioToolbox CoreAudio

tvos:
	ADDON_LIBS = libs/steamaudio/lib/tvos/libphonon.a
	ADDON_LDFLAGS = -lz
	ADDON_FRAMEWORKS = Accelerate AudioToolbox CoreAudio

# ----- Android (PG picks ABI subfolder via additional config / project settings) -----
android:
	ADDON_LIBS = libs/steamaudio/lib/android/arm64/libphonon.so
	ADDON_LIBS += libs/steamaudio/lib/android/armeabi-v7a/libphonon.so

# ----- Web -----
emscripten:
	ADDON_LIBS = libs/steamaudio/lib/emscripten/WASM/libphonon.a
