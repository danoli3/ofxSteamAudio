#!/usr/bin/env bash
# Download prebuilt Steam Audio libs for ofxSteamAudio from a GitHub Release tag.
#
# Usage:
#   ./scripts/download_libs.sh [package_id|all|host]
#   LIBS_TAG=libs-v4.8.1 REPO=owner/name ./scripts/download_libs.sh osx
#
# Env:
#   LIBS_TAG   Release tag (default: libs-v${STEAMAUDIO_VER})
#   REPO       owner/repo (default: detect from git remote, else ofxSteamAudio)
#   FORCE=1    Overwrite existing platform libs

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ADDON_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
# shellcheck source=platforms.sh
source "$SCRIPT_DIR/platforms.sh"

DEST_ROOT="${ADDON_ROOT}/libs/steamaudio"
TMP_DIR="${ADDON_ROOT}/scripts/.cache/download"
FORCE="${FORCE:-0}"

detect_repo() {
  if [[ -n "${REPO:-}" ]]; then
    echo "$REPO"
    return
  fi
  local url
  url="$(git -C "$ADDON_ROOT" remote get-url origin 2>/dev/null || true)"
  if [[ "$url" =~ github.com[:/]([^/]+)/([^/.]+) ]]; then
    echo "${BASH_REMATCH[1]}/${BASH_REMATCH[2]}"
    return
  fi
  echo "openframeworks/ofxSteamAudio"
}

download_url_for() {
  local repo="$1" tag="$2" asset="$3"
  echo "https://github.com/${repo}/releases/download/${tag}/${asset}"
}

verify_platform() {
  local id="$1"
  local of_path lib_name lib_dir
  of_path="$(platform_field "$id" of_path)" || return 1
  lib_name="$(expected_lib_name "$id")"
  lib_dir="${DEST_ROOT}/lib/${of_path}"
  if [[ -f "${lib_dir}/${lib_name}" ]]; then
    return 0
  fi
  # Accept shared lib on linux if static missing
  if [[ -f "${lib_dir}/libphonon.so" ]]; then
    return 0
  fi
  return 1
}

extract_asset() {
  local archive="$1"
  mkdir -p "$DEST_ROOT"
  # Archives are expected to contain include/, lib/, license/ relative to steamaudio root
  tar -xzf "$archive" -C "$DEST_ROOT" --strip-components=0 2>/dev/null || \
  tar -xzf "$archive" -C "$(dirname "$DEST_ROOT")"
}

download_one() {
  local id="$1"
  local repo tag asset url of_path
  if ! platform_field "$id" id >/dev/null; then
    echo "error: unknown package_id '$id'" >&2
    echo "Known ids:" >&2
    platform_ids | sed 's/^/  /' >&2
    return 1
  fi

  of_path="$(platform_field "$id" of_path)"
  if [[ "$FORCE" != "1" ]] && verify_platform "$id"; then
    echo "[ok] $id already present at lib/${of_path} (FORCE=1 to redownload)"
    return 0
  fi

  repo="$(detect_repo)"
  tag="$LIBS_TAG"
  asset="$(asset_name "$id")"
  url="$(download_url_for "$repo" "$tag" "$asset")"

  mkdir -p "$TMP_DIR"
  local out="${TMP_DIR}/${asset}"
  echo "==> Downloading $asset"
  echo "    $url"

  if command -v curl >/dev/null 2>&1; then
    if ! curl -fL --retry 3 -o "$out" "$url"; then
      echo "error: download failed for $id" >&2
      echo "  Tag:    $tag" >&2
      echo "  Repo:   $repo" >&2
      echo "  Asset:  $asset" >&2
      echo "  Build with:  ./scripts/manage_libs.sh build $id" >&2
      echo "  Or package+upload a release asset named $asset" >&2
      return 1
    fi
  elif command -v wget >/dev/null 2>&1; then
    wget -O "$out" "$url"
  else
    echo "error: need curl or wget" >&2
    return 1
  fi

  echo "==> Extracting into libs/steamaudio/"
  # Clear only this platform's lib dir
  rm -rf "${DEST_ROOT}/lib/${of_path}"
  mkdir -p "${DEST_ROOT}/lib/${of_path}"

  local extract_tmp="${TMP_DIR}/extract_${id}"
  rm -rf "$extract_tmp"
  mkdir -p "$extract_tmp"
  tar -xzf "$out" -C "$extract_tmp"

  # Flexible layout: either steamaudio/{include,lib,license} or {include,lib,license}
  if [[ -d "$extract_tmp/steamaudio" ]]; then
    extract_tmp="$extract_tmp/steamaudio"
  fi
  if [[ -d "$extract_tmp/libs/steamaudio" ]]; then
    extract_tmp="$extract_tmp/libs/steamaudio"
  fi

  if [[ -d "$extract_tmp/include" ]]; then
    mkdir -p "$DEST_ROOT/include"
    cp -R "$extract_tmp/include/." "$DEST_ROOT/include/"
  fi
  if [[ -d "$extract_tmp/license" ]]; then
    mkdir -p "$DEST_ROOT/license"
    cp -R "$extract_tmp/license/." "$DEST_ROOT/license/"
  fi
  if [[ -d "$extract_tmp/lib" ]]; then
    mkdir -p "$DEST_ROOT/lib"
    cp -R "$extract_tmp/lib/." "$DEST_ROOT/lib/"
  else
    # Flat: files at root for this platform
    cp -R "$extract_tmp/." "${DEST_ROOT}/lib/${of_path}/" 2>/dev/null || true
  fi

  echo "${STEAMAUDIO_VER}" > "${DEST_ROOT}/VERSION"
  echo "${tag}" > "${DEST_ROOT}/LIBS_TAG"
  echo "${id}" >> "${DEST_ROOT}/INSTALLED_PLATFORMS"
  sort -u "${DEST_ROOT}/INSTALLED_PLATFORMS" -o "${DEST_ROOT}/INSTALLED_PLATFORMS" 2>/dev/null || true

  if verify_platform "$id"; then
    echo "[ok] installed $id → libs/steamaudio/lib/${of_path}"
  else
    echo "warning: extract finished but expected lib not found for $id" >&2
    echo "  looked for lib/${of_path}/$(expected_lib_name "$id")" >&2
    find "${DEST_ROOT}/lib" -maxdepth 4 -type f 2>/dev/null | head -40 >&2 || true
    return 1
  fi
}

main() {
  local target="${1:-host}"
  case "$target" in
    -h|--help)
      cat <<EOF
download_libs.sh — fetch ofxSteamAudio prebuilt binaries

Usage:
  $0 [host|all|<package_id>]

Env:
  LIBS_TAG=libs-v${STEAMAUDIO_VER}
  REPO=owner/ofxSteamAudio
  FORCE=1

Platforms:
$(list_platforms_table)
EOF
      exit 0
      ;;
    host)
      target="$(detect_host_package_id)"
      download_one "$target"
      ;;
    all)
      local id sup fail=0
      for id in $(platform_ids); do
        sup="$(platform_field "$id" support)"
        # Download full + experimental by default for "all"; skip planned
        if [[ "$sup" == "planned" ]]; then
          echo "[skip] $id (planned)"
          continue
        fi
        download_one "$id" || fail=1
      done
      exit $fail
      ;;
    *)
      download_one "$target"
      ;;
  esac
}

main "$@"
