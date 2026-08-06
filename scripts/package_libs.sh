#!/usr/bin/env bash
# Package installed Steam Audio libs into release-ready tar.gz assets.
#
# Usage:
#   ./scripts/package_libs.sh [package_id|host|all]
# Writes to scripts/dist/

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ADDON_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
# shellcheck source=platforms.sh
source "$SCRIPT_DIR/platforms.sh"

SRC="${ADDON_ROOT}/libs/steamaudio"
DIST="${ADDON_ROOT}/scripts/dist"
mkdir -p "$DIST"

package_one() {
  local id="$1"
  local of_path asset stage
  if ! platform_field "$id" id >/dev/null; then
    echo "error: unknown package_id '$id'" >&2
    return 1
  fi
  of_path="$(platform_field "$id" of_path)"
  if [[ ! -d "${SRC}/lib/${of_path}" ]]; then
    echo "error: missing libs/steamaudio/lib/${of_path} — build or download first" >&2
    return 1
  fi

  asset="$(asset_name "$id")"
  stage="${DIST}/stage_${id}"
  rm -rf "$stage"
  mkdir -p "$stage/steamaudio/lib/${of_path}"
  mkdir -p "$stage/steamaudio/include"
  mkdir -p "$stage/steamaudio/license"

  cp -R "${SRC}/lib/${of_path}/." "$stage/steamaudio/lib/${of_path}/"
  if [[ -d "${SRC}/include" ]]; then
    cp -R "${SRC}/include/." "$stage/steamaudio/include/"
  fi
  if [[ -d "${SRC}/license" ]]; then
    cp -R "${SRC}/license/." "$stage/steamaudio/license/"
  elif [[ -f "${ADDON_ROOT}/libs/LICENSE.md" ]]; then
    cp "${ADDON_ROOT}/libs/LICENSE.md" "$stage/steamaudio/license/"
  fi

  # Metadata
  {
    echo "steamaudio_version=${STEAMAUDIO_VER}"
    echo "package_id=${id}"
    echo "of_path=${of_path}"
    echo "built_utc=$(date -u +%Y-%m-%dT%H:%M:%SZ)"
    echo "build_id=${BUILD_ID}"
  } > "$stage/steamaudio/PACKAGE.txt"

  tar -czf "${DIST}/${asset}" -C "$stage" steamaudio
  rm -rf "$stage"
  local sz
  sz="$(du -h "${DIST}/${asset}" | awk '{print $1}')"
  echo "[ok] ${DIST}/${asset} (${sz})"
}

main() {
  local target="${1:-host}"
  case "$target" in
    -h|--help)
      echo "Usage: $0 [host|all|<package_id>]"
      exit 0
      ;;
    host) package_one "$(detect_host_package_id)" ;;
    all)
      local id of_path
      for id in $(platform_ids); do
        of_path="$(platform_field "$id" of_path)"
        if [[ -d "${SRC}/lib/${of_path}" ]]; then
          package_one "$id" || true
        else
          echo "[skip] $id (not installed)"
        fi
      done
      echo "Packages in $DIST:"
      ls -la "$DIST"/*.tar.gz 2>/dev/null || true
      ;;
    *) package_one "$target" ;;
  esac
}

main "$@"
