#!/usr/bin/env bash
#
# Steam Audio (Phonon) - official spatial audio SDK from Valve
# https://github.com/valvesoftware/steam-audio
# C API library: libphonon (static by default in this formula)

FORMULA_TYPES=("osx" "vs" "ios" "watchos" "catos" "xros" "tvos" "android" "emscripten" "linux")
FORMULA_DEPENDS=("zlib")

# define the version
VER=4.8.1
BUILD_ID=1
DEFINES=""

GIT_URL=https://github.com/valvesoftware/steam-audio
GIT_TAG=v${VER}
URL=https://github.com/valvesoftware/steam-audio/archive/refs/tags/v${VER}

# download the source code and unpack it into LIB_NAME
function download() {
    . "$DOWNLOADER_SCRIPT"

    if [ "$TYPE" == "vs" ]; then
        downloader "${URL}.zip"
        unzip -q "v${VER}.zip"
        mv "steam-audio-${VER}" steamaudio
        rm "v${VER}.zip"
    else
        downloader "${URL}.tar.gz"
        tar -xf "v${VER}.tar.gz"
        mv "steam-audio-${VER}" steamaudio
        rm "v${VER}.tar.gz"
    fi
}

# prepare the build environment, executed inside the lib src dir (steamaudio/)
function prepare() {
    # Steam Audio handles its own third-party dependencies (minimal set)
    echo "=== SteamAudio prepare: fetching minimal dependencies ==="
    # pushd core/build > /dev/null || {
    #     echoError "core/build directory not found - check download"
    #     return 1
    # }

    # Map Apothecary TYPE to SteamAudio platform (official support: windows, linux, osx, android, ios)
    # Unsupported platforms (watchos/catos/xros/tvos/emscripten) will fall back or may require manual tweaks
    PLATFORM_NAME="unknown"
    case "$TYPE" in
        "osx") PLATFORM_NAME="osx" ;;
        "vs") PLATFORM_NAME="windows" ;;
        "ios"|"watchos"|"catos"|"xros"|"tvos") PLATFORM_NAME="ios" ;;  # iOS toolchain often works for other Apple embedded
        "android") PLATFORM_NAME="android" ;;
        "linux") PLATFORM_NAME="linux" ;;
        "emscripten") PLATFORM_NAME="wasm" ;;  # experimental - wasm support exists in some builds
        *) PLATFORM_NAME="$TYPE" ;;
    esac

    cd core/build


    python3 get_dependencies.py -p$PLATFORM_NAME

    # popd > /dev/null

    echo "=== SteamAudio prepare complete ==="
}

function load() {
    . "$LOAD_SCRIPT"
    LOAD_RESULT=$(loadsave ${TYPE} "steamaudio" ${ARCH} ${VER} "$LIBS_DIR_REAL/steamaudio/lib/$TYPE/$PLATFORM" ${BUILD_ID})
    PREBUILT=$(echo "$LOAD_RESULT" | tail -n 1)
    if [ "$PREBUILT" -eq 1 ]; then
        echo 1
    else
        echo 0
    fi
}

# executed inside the lib src dir (steamaudio/)
function build() {
    LIBS_ROOT=$(realpath $LIBS_DIR)

    if [[ $FORCE_DOWNLOAD -eq 0 ]] && [[ $USE_SAVE == 1 ]]; then
        result=$(load "steamaudio" | tail -n 1)
        echoInfo "===Build $1 - Checking if Precompiled binary :[$result]==="
        if [ $result -eq 1 ]; then
            echoInfo "===Build \"$1\" Precompiled binary validated. Skipping updateFormula==="
            return 0
        else
            echoInfo "===Build Precompiled not found or outdated. Continue updateFormula for \"$1\"=== "
        fi
    else
        echoInfo "===Build  Not using cache : [FORCE_DOWNLOAD=$FORCE_DOWNLOAD] [USE_SAVE=$USE_SAVE == 1] for updateFormula \"$1\" ==="
    fi

    DEPS_PATH="../deps"

    # Core CMake flags (static library, minimal features, disable everything non-essential)
    export DEFINES="
        -DCMAKE_C_STANDARD=${C_STANDARD} \
        -DCMAKE_CXX_STANDARD=${CPP_STANDARD} \
        -DCMAKE_CXX_STANDARD_REQUIRED=ON \
        -DCMAKE_CXX_EXTENSIONS=OFF \
        -DBUILD_SHARED_LIBS=OFF \
        -DSTEAMAUDIO_BUILD_TESTS=OFF \
        -DSTEAMAUDIO_BUILD_ITESTS=OFF \
        -DSTEAMAUDIO_BUILD_BENCHMARKS=OFF \
        -DSTEAMAUDIO_BUILD_SAMPLES=OFF \
        -DSTEAMAUDIO_BUILD_DOCS=OFF \
        -DSTEAMAUDIO_ENABLE_IPP=OFF \
        -DSTEAMAUDIO_ENABLE_EMBREE=OFF \
        -DSTEAMAUDIO_ENABLE_RADEONRAYS=OFF \
        -DSTEAMAUDIO_ENABLE_TRUENEXT=OFF \
        -DSTEAMAUDIO_ENABLE_AVX=OFF \
        -DCMAKE_POSITION_INDEPENDENT_CODE=ON"

    # Build directory inside core (the actual CMake project is in core/)
    OUTPUT_DIR="build_${TYPE}_${PLATFORM}"
    mkdir -p "core/${OUTPUT_DIR}"
    cd "core/${OUTPUT_DIR}"

    rm -f CMakeCache.txt *.a *.o *.lib *.so *.dylib 2>/dev/null || true

    # Platform-specific configuration
    if [[ "$TYPE" =~ ^(osx|ios|watchos|catos|xros|tvos)$ ]]; then
        echoInfo "Building SteamAudio for Apple platform: $TYPE / $PLATFORM"

        ZLIB_ROOT="$LIBS_ROOT/zlib/"
        ZLIB_INCLUDE_DIR="$LIBS_ROOT/zlib/include"
        ZLIB_LIBRARY="$LIBS_ROOT/zlib/lib/$TYPE/$PLATFORM/zlib.a"

        PFFFT_ROOT="$LIBS_ROOT/pffft/"
        PFFFT_INCLUDE_DIR="$DEPS_PATH/pffft/include/pffft"
        PFFFT_LIBRARY="$DEPS_PATH/pffft/lib/$TYPE/release/libpffft.a"

        MYSOTA_ROOT="$LIBS_ROOT/mysofa/"
        MYSOTA_INCLUDE_DIR="$DEPS_PATH/mysofa/include"
        MYSOTA_LIBRARY="$DEPS_PATH/mysofa/lib/$TYPE/release/libmysofa.a"

        export PKG_CONFIG_PATH="/usr/local/lib/pkgconfig:${PKG_CONFIG_PATH}:${ZLIB_ROOT}/lib/$TYPE/$PLATFORM"


        cmake ../ \
            ${DEFINES} \
            -DCMAKE_MODULE_PATH="../../build" \
            -DCMAKE_PREFIX_PATH="../../deps" \
            -DCMAKE_CXX_FLAGS="-DUSE_PTHREADS=1 ${FLAG_RELEASE} -Wno-error=implicit-function-declaration" \
            -DCMAKE_C_FLAGS="-DUSE_PTHREADS=1 ${FLAG_RELEASE} -Wno-error=implicit-function-declaration" \
            -DCMAKE_TOOLCHAIN_FILE=$APOTHECARY_DIR/toolchains/ios.toolchain.cmake \
            -DPLATFORM=$PLATFORM \
            -DCMAKE_BUILD_TYPE=Release \
            -DCMAKE_INSTALL_PREFIX=Release \
            -DCMAKE_CXX_FLAGS="${FLAG_RELEASE}" \
            -DCMAKE_C_FLAGS="${FLAG_RELEASE}" \
            -DCMAKE_IGNORE_PATH=/opt/homebrew \
            -DCMAKE_FIND_PACKAGE_NO_PACKAGE_REGISTRY=ON \
            -DCMAKE_VERBOSE_MAKEFILE=${VERBOSE_MAKEFILE} \
            -DDEPLOYMENT_TARGET=${MIN_SDK_VER} \
            -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
            -DCMAKE_MINIMUM_REQUIRED_VERSION=3.22 \
            -DDEPLOYMENT_TARGET=${MIN_SDK_VER} \
            -DPFFFT_ROOT=${PFFFT_ROOT} \
            -DPFFFT_INCLUDE_DIR=${PFFFT_INCLUDE_DIR} \
            -DPFFFT_LIBRARY=${PFFFT_LIBRARY} \
            -DENABLE_BITCODE=OFF \
            -DENABLE_ARC=OFF \
            -DENABLE_VISIBILITY=OFF

        cmake --build . --config Release -j${PARALLEL_MAKE} --target install

    elif [[ "$TYPE" == "linux" ]]; then
        echoInfo "Building SteamAudio for Linux: $TYPE / $PLATFORM"

        if [ $CROSSCOMPILING -eq 1 ]; then
            source $APOTHECARY_DIR/configure/${TYPE}${PLATFORM}_configure.sh
        fi

        cmake ../ \
            ${DEFINES} \
            -DCMAKE_TOOLCHAIN_FILE=$APOTHECARY_DIR/toolchains/${TYPE}${PLATFORM}.toolchain.cmake \
            -DGCC_VERSION=${GCC_VERSION} \
            -DCMAKE_MODULE_PATH="../../build" \
            -DCMAKE_PREFIX_PATH="../../deps" \
            -DCMAKE_SYSTEM_PROCESSOR=$ABI \
            -DPLATFORM=$PLATFORM \
            -DCMAKE_BUILD_TYPE=Release \
            -DCMAKE_INSTALL_PREFIX=Release \
            -DCMAKE_CXX_FLAGS="${FLAG_RELEASE}" \
            -DCMAKE_C_FLAGS="${FLAG_RELEASE}" \
            -DCMAKE_VERBOSE_MAKEFILE=${VERBOSE_MAKEFILE}

        cmake --build . --config Release -j${PARALLEL_MAKE} --target install

    elif [ "$TYPE" == "vs" ]; then
        echoInfo "Building SteamAudio for Windows VS: $ARCH / $VS_VER"

        GENERATOR_NAME="Visual Studio ${VS_VER_GEN}"
        PLATFORM_NAME="x64"  # adjust if needed for arm64 etc.

        cmake ../ \
            ${DEFINES} \
            -A "${PLATFORM}" \
            -G "${GENERATOR_NAME}" \
            ${CMAKE_WIN_SDK} \
            -DCMAKE_MODULE_PATH="../../build" \
            -DCMAKE_PREFIX_PATH="../../deps" \
            -DCMAKE_BUILD_TYPE=Release \
            -DCMAKE_INSTALL_PREFIX=Release \
            -DCMAKE_CXX_FLAGS_RELEASE="${VS_C_FLAGS} ${FLAGS_RELEASE}" \
            -DCMAKE_C_FLAGS_RELEASE="${VS_C_FLAGS} ${FLAGS_RELEASE}" \
            -DCMAKE_VERBOSE_MAKEFILE=${VERBOSE_MAKEFILE}

        cmake --build . --config Release -j${PARALLEL_MAKE} --target install

    elif [ "$TYPE" == "android" ]; then
        echoInfo "Building SteamAudio for Android: $ABI"

        source $APOTHECARY_DIR/configure/android_configure.sh $ABI cmake

        cmake ../ \
            ${DEFINES} \
            -DCMAKE_TOOLCHAIN_FILE=$APOTHECARY_DIR/toolchains/android.toolchain.cmake \
            -DPLATFORM=$PLATFORM \
            -DANDROID_PLATFORM=${ANDROID_PLATFORM} \
            -DANDROID_ABI=${ABI} \
            -DANDROID_API=${ANDROID_API} \
            -DCMAKE_MODULE_PATH="../../build" \
            -DCMAKE_PREFIX_PATH="../../deps" \
            -DANDROID_TOOLCHAIN=clang \
            -DANDROID_NDK_ROOT=$ANDROID_NDK_ROOT \
            -DCMAKE_BUILD_TYPE=Release \
            -DCMAKE_INSTALL_PREFIX=Release \
            -DCMAKE_CXX_FLAGS="-fvisibility-inlines-hidden -std=c++${CPP_STANDARD} ${FLAG_RELEASE}" \
            -DCMAKE_C_FLAGS="-fvisibility-inlines-hidden -std=c${C_STANDARD} ${FLAG_RELEASE}" \
            -DCMAKE_VERBOSE_MAKEFILE=${VERBOSE_MAKEFILE}

        cmake --build . --config Release -j${PARALLEL_MAKE} --target install

    elif [ "$TYPE" == "emscripten" ]; then
        echoInfo "Building SteamAudio for Emscripten (experimental wasm)"

        ZLIB_ROOT="$LIBS_ROOT/zlib/"
        ZLIB_INCLUDE_DIR="$LIBS_ROOT/zlib/include"
        ZLIB_LIBRARY="$LIBS_ROOT/zlib/lib/$TYPE/$PLATFORM/zlib.a"

        PFFFT_ROOT="$DEPS_PATH/pffft/"
        PFFFT_INCLUDE_DIR="$DEPS_PATH/pffft/include/pffft"
        PFFFT_LIBRARY="$DEPS_PATH/pffft/lib/$TYPE/release/libpffft.a"

        MySOFA_ROOT="$DEPS_PATH/mysofa/"
        MySOFA_INCLUDE_DIR="$DEPS_PATH/mysofa/include"
        MySOFA_LIBRARY="$DEPS_PATH/mysofa/lib/$TYPE/release/libmysofa.a"

        FlatBuffers_INCLUDE_DIR="$DEPS_PATH/flatbuffers/include"

        $EMSDK/upstream/emscripten/emcmake cmake ../ \
            ${DEFINES} \
            -DCMAKE_TOOLCHAIN_FILE=$EMSDK/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake \
            -DEMSCRIPTEN=ON \
            -DCMAKE_BUILD_TYPE=Release \
            -DCMAKE_INSTALL_PREFIX=Release \
            -DCMAKE_MODULE_PATH="../../build" \
            -DCMAKE_PREFIX_PATH="../../deps" \
            -DPFFFT_ROOT=${PFFFT_ROOT} \
            -DPFFFT_INCLUDE_DIR=${PFFFT_INCLUDE_DIR} \
            -DPFFFT_LIBRARY=${PFFFT_LIBRARY} \
            -DZLIB_ROOT=${ZLIB_ROOT} \
            -DZLIB_INCLUDE_DIR=${ZLIB_INCLUDE_DIR} \
            -DZLIB_LIBRARY=${ZLIB_LIBRARY} \
            -DMySOFA_ROOT=${MySOFA_ROOT} \
            -DMySOFA_INCLUDE_DIR=${MySOFA_INCLUDE_DIR} \
            -DMySOFA_LIBRARY=${MySOFA_LIBRARY} \
            -DFlatBuffers_INCLUDE_DIR=${FlatBuffers_INCLUDE_DIR} \
            -DCMAKE_IGNORE_PATH=/opt/homebrew \
            -DCMAKE_FIND_PACKAGE_NO_PACKAGE_REGISTRY=ON \
            -DCMAKE_CXX_FLAGS="-std=c++${CPP_STANDARD} ${FLAG_RELEASE}" \
            -DCMAKE_C_FLAGS="-std=c${C_STANDARD} ${FLAG_RELEASE}" \
            -DCMAKE_VERBOSE_MAKEFILE=${VERBOSE_MAKEFILE}

        $EMSDK/upstream/emscripten/emmake make -j${PARALLEL_MAKE}
        $EMSDK/upstream/emscripten/emmake make install
    fi

}

# executed inside the lib src dir, first arg $1 is the dest libs dir root
function copy() {
    mkdir -p $1/include
    . "$SECURE_SCRIPT"
    echo "copy"

    # The install target puts libraries in bin/ (or Release/ subdir depending on generator)
    # Steam Audio C API library name is libphonon.a (Unix/macOS/iOS/Android/Linux) or phonon.lib (Windows)
    LIB_SRC_DIR="core/build_${TYPE}_${PLATFORM}/Release"  # adjust if your build dir naming differs
    if [ ! -d "${LIB_SRC_DIR}" ]; then
        LIB_SRC_DIR="core/build_${TYPE}_${PLATFORM}"  # fallback
    fi

    if [ "$TYPE" == "vs" ]; then
        mkdir -p $1/lib/$TYPE/$PLATFORM/
        # Windows static lib is usually phonon.lib (or phonons.lib in some older builds)
        cp -v "${LIB_SRC_DIR}/phonon.lib" $1/lib/$TYPE/$PLATFORM/phonon.lib 2>/dev/null || \
        cp -v "${LIB_SRC_DIR}/libphonon.lib" $1/lib/$TYPE/$PLATFORM/phonon.lib || \
        echoWarning "Windows library not found at expected location - check build output"
        secure $1/lib/$TYPE/$PLATFORM/phonon.lib
        secure "$1/lib/$TYPE/$PLATFORM/phonon.lib" "steamaudio.pkl" "$VERSION" "$DEFINES" "$BUILD_ID" "$FORMULA_DEPENDS"
    elif [ "$TYPE" == "emscripten" ]; then
        mkdir -p $1/lib/$TYPE/$PLATFORM/
        cp -v "${LIB_SRC_DIR}/lib/wasm/libphonon.a" $1/lib/$TYPE/$PLATFORM/libphonon.a 2>/dev/null || \
        cp -v "${LIB_SRC_DIR}/libphonon.a" $1/lib/$TYPE/$PLATFORM/libphonon.a || \
        echoWarning "libphonon.a not found - check build output"
        secure "$1/lib/$TYPE/$PLATFORM/libphonon.a" "steamaudio.pkl" "$VERSION" "$DEFINES" "$BUILD_ID" "$FORMULA_DEPENDS"
    else
        mkdir -p $1/lib/$TYPE/$PLATFORM/
        cp -v "${LIB_SRC_DIR}/lib/$TYPE/libphonon.a" $1/lib/$TYPE/$PLATFORM/libphonon.a 2>/dev/null || \
        cp -v "${LIB_SRC_DIR}/libphonon.a" $1/lib/$TYPE/$PLATFORM/libphonon.a || \
        echoWarning "libphonon.a not found - check build output"
        secure "$1/lib/$TYPE/$PLATFORM/libphonon.a" "steamaudio.pkl" "$VERSION" "$DEFINES" "$BUILD_ID" "$FORMULA_DEPENDS"
    fi

    # Headers are in core/include/ (phonon.h + others)
    cp -R ${LIB_SRC_DIR}/include/* $1/include/ 2>/dev/null || \
    echoWarning "Headers not found at expected location"

    # copy license
    if [ -d "$1/license" ]; then
        rm -rf $1/license
    fi
    mkdir -p $1/license
    cp -v LICENSE.md $1/license/ 2>/dev/null || cp -v LICENSE $1/license/ || true
    cp -v ${LIB_SRC_DIR}/root/THIRDPARTY.md $1/license/ 2>/dev/null || true
    echo "=== SteamAudio copy complete for $TYPE/$PLATFORM ==="
}

# executed inside the lib src dir
function clean() {
    if [ "$TYPE" == "vs" ] || [ "$TYPE" == "android" ] || [[ "$TYPE" =~ ^(osx|ios|watchos|catos|xros|tvos|emscripten|linux)$ ]]; then
        rm -rf core/build_${TYPE}_${PLATFORM} 2>/dev/null || true
        rm -rf core/build 2>/dev/null || true  # clean Python build artifacts if present
    else
        make clean 2>/dev/null || true
    fi
    echo "=== SteamAudio clean complete ==="
}
