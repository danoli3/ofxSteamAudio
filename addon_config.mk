ADDON_NAME = ofxSteamAudio

ADDON_INCLUDES = libs/include

# Platform libs (download Steam Audio 4.8.1 SDK from https://valvesoftware.github.io/steam-audio/)
ifeq ($(PLATFORM),linux64)
    ADDON_LIBS = libs/lib/linux64/libphonon.so
else ifeq ($(PLATFORM),osx)
    ADDON_LIBS = libs/lib/osx/libphonon.a
else ifeq ($(PLATFORM),emscripten)
    ADDON_LIBS = libs/lib/emscripten/WASM/libphonon.a
else ifeq ($(PLATFORM),msvc)
    ADDON_LIBS = libs/lib/win64/phonon.lib
endif

ADDON_DEFINES = STEAMAUDIO_VERSION=481
