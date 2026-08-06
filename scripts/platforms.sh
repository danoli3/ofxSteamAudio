#!/usr/bin/env bash
# Platform matrix for ofxSteamAudio binaries.
# Intersection of openFrameworks + Steam Audio (Phonon) support, plus OF-only
# Apple targets that use the same apothecary formula path.
#
# shellcheck disable=SC2034

STEAMAUDIO_VER="${STEAMAUDIO_VER:-4.8.1}"
LIBS_TAG="${LIBS_TAG:-libs-v${STEAMAUDIO_VER}}"
BUILD_ID="${BUILD_ID:-1}"
ASSET_PREFIX="steamaudio-${STEAMAUDIO_VER}"

# package_id|of_lib_subdir|apothecary_type|arch|support|ci_runner|description
# support: full | experimental | planned
# ci_runner: GitHub-hosted runner label, or "none" if not runnable on GHA yet
PLATFORM_ROWS=(
  # Desktop
  "osx|osx|osx|arm64|full|macos-14|macOS (host arch via apothecary)"
  "linux64|linux64|linux|64|full|ubuntu-22.04|Linux x86_64"
  "linuxaarch64|linuxaarch64|linux|arm64|full|ubuntu-24.04-arm|Linux ARM64 (aarch64)"
  "linuxarmv7l|linuxarmv7l|linux|armv7|experimental|none|Linux ARMv7 (needs cross/self-hosted)"
  "vs-x64|vs/x64|vs|x64|full|windows-2022|Windows MSVC x64"
  "vs-arm64|vs/arm64|vs|arm64|experimental|windows-11-arm|Windows MSVC ARM64"
  "vs-x86|vs/x86|vs|Win32|experimental|windows-2022|Windows MSVC x86"
  "msys2|msys2|msys2|64|experimental|windows-2022|MSYS2 / MinGW"
  # Mobile
  "ios|ios|ios|arm64|full|macos-14|iOS device arm64"
  "ios-sim|ios/simulator|ios|x86_64|experimental|macos-14|iOS simulator"
  "tvos|tvos|tvos|arm64|experimental|macos-14|tvOS"
  "android-arm64|android/arm64|android|arm64|full|ubuntu-22.04|Android arm64-v8a"
  "android-armv7|android/armeabi-v7a|android|armv7|full|ubuntu-22.04|Android armeabi-v7a"
  "android-x86|android/x86|android|x86|experimental|ubuntu-22.04|Android x86"
  "android-x86_64|android/x86_64|android|x86_64|experimental|ubuntu-22.04|Android x86_64"
  # Web
  "emscripten-wasm|emscripten/WASM|emscripten|64|experimental|ubuntu-22.04|Emscripten wasm32"
  "emscripten-wasm64|emscripten/WASM64|emscripten|64|experimental|ubuntu-22.04|Emscripten wasm64"
  # Extra Apple
  "watchos|watchos|watchos|arm64|planned|macos-14|watchOS"
  "xros|xros|xros|arm64|planned|macos-14|visionOS"
  "catos|catos|catos|arm64|planned|macos-14|Mac Catalyst"
)

platform_ids() {
  local row
  for row in "${PLATFORM_ROWS[@]}"; do
    echo "${row%%|*}"
  done
}

# Platforms that have a GHA runner (ci_runner != none)
platform_ids_ci() {
  local row id ofp apo arch sup runner desc
  for row in "${PLATFORM_ROWS[@]}"; do
    IFS='|' read -r id ofp apo arch sup runner desc <<<"$row"
    if [[ "$runner" != "none" ]]; then
      echo "$id"
    fi
  done
}

platform_field() {
  local want="$1" field="$2" row id ofp apo arch sup runner desc
  for row in "${PLATFORM_ROWS[@]}"; do
    IFS='|' read -r id ofp apo arch sup runner desc <<<"$row"
    if [[ "$id" == "$want" ]]; then
      case "$field" in
        id) echo "$id" ;;
        of_path) echo "$ofp" ;;
        apo_type) echo "$apo" ;;
        arch) echo "$arch" ;;
        support) echo "$sup" ;;
        ci_runner) echo "$runner" ;;
        desc) echo "$desc" ;;
        *) return 1 ;;
      esac
      return 0
    fi
  done
  return 1
}

detect_host_package_id() {
  local uname_s uname_m
  uname_s="$(uname -s 2>/dev/null || echo unknown)"
  uname_m="$(uname -m 2>/dev/null || echo unknown)"
  case "$uname_s" in
    Darwin) echo "osx" ;;
    Linux)
      case "$uname_m" in
        x86_64|amd64) echo "linux64" ;;
        aarch64|arm64) echo "linuxaarch64" ;;
        armv7l|armv7) echo "linuxarmv7l" ;;
        *) echo "linux64" ;;
      esac
      ;;
    MINGW*|MSYS*|CYGWIN*) echo "vs-x64" ;;
    *) echo "osx" ;;
  esac
}

list_platforms_table() {
  local row id ofp apo arch sup runner desc
  printf "%-20s %-12s %-16s %-22s %s\n" "PACKAGE_ID" "SUPPORT" "CI_RUNNER" "OF lib/ PATH" "DESCRIPTION"
  printf "%-20s %-12s %-16s %-22s %s\n" "----------" "-------" "---------" "-----------" "-----------"
  for row in "${PLATFORM_ROWS[@]}"; do
    IFS='|' read -r id ofp apo arch sup runner desc <<<"$row"
    printf "%-20s %-12s %-16s %-22s %s\n" "$id" "$sup" "$runner" "$ofp" "$desc"
  done
}

expected_lib_name() {
  local id="$1"
  case "$id" in
    vs-*|msys2) echo "phonon.lib" ;;
    android-*) echo "libphonon.so" ;;
    linux*) echo "libphonon.a" ;;
    *) echo "libphonon.a" ;;
  esac
}

asset_name() {
  local id="$1"
  echo "${ASSET_PREFIX}-${id}.tar.gz"
}

# Emit GitHub Actions matrix JSON for build-libs (ci_runner != none)
# Optional filter: full | experimental | planned | all (default all with runner)
emit_gha_matrix_json() {
  local filter="${1:-all}"
  local row id ofp apo arch sup runner desc first=1
  echo -n '{"include":['
  for row in "${PLATFORM_ROWS[@]}"; do
    IFS='|' read -r id ofp apo arch sup runner desc <<<"$row"
    [[ "$runner" == "none" ]] && continue
    if [[ "$filter" != "all" && "$sup" != "$filter" ]]; then
      continue
    fi
    # planned only if FORCE_PLANNED or filter=planned|all
    if [[ "$sup" == "planned" && "$filter" == "full" ]]; then
      continue
    fi
    [[ $first -eq 1 ]] || echo -n ','
    first=0
    printf '{"package_id":"%s","os":"%s","apo_type":"%s","support":"%s"}' \
      "$id" "$runner" "$apo" "$sup"
  done
  echo ']}'
}
