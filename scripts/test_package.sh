#!/usr/bin/env bash
# Verify a packaged or installed Steam Audio platform tree.
# Does NOT need openFrameworks — only checks archive layout + expected lib files.
#
# Usage:
#   ./scripts/test_package.sh <package_id> [path/to/steamaudio-VER-id.tar.gz]
#   ./scripts/test_package.sh osx                          # uses installed libs/
#   ./scripts/test_package.sh osx scripts/dist/foo.tar.gz  # uses archive
#
# Exit 0 = pass, 1 = fail. Writes lines to stdout; optional STATUS_FILE for JSON line.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ADDON_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
# shellcheck source=platforms.sh
source "$SCRIPT_DIR/platforms.sh"

ID="${1:-}"
ARCHIVE="${2:-}"
STATUS_FILE="${STATUS_FILE:-}"
TMP=""

cleanup() {
  if [[ -n "${TMP:-}" && -d "$TMP" ]]; then
    rm -rf "$TMP" || true
  fi
  return 0
}
trap cleanup EXIT

if [[ -z "$ID" || "$ID" == "-h" || "$ID" == "--help" ]]; then
  echo "Usage: $0 <package_id> [archive.tar.gz]"
  exit 2
fi

if ! platform_field "$ID" id >/dev/null; then
  echo "FAIL unknown package_id=$ID"
  exit 1
fi

OF_PATH="$(platform_field "$ID" of_path)"
LIB_NAME="$(expected_lib_name "$ID")"
SUPPORT="$(platform_field "$ID" support)"
ROOT=""

if [[ -n "$ARCHIVE" ]]; then
  if [[ ! -f "$ARCHIVE" ]]; then
    echo "FAIL archive missing: $ARCHIVE"
    exit 1
  fi
  TMP="$(mktemp -d)"
  tar -xzf "$ARCHIVE" -C "$TMP"
  if [[ -d "$TMP/steamaudio" ]]; then
    ROOT="$TMP/steamaudio"
  elif [[ -d "$TMP/libs/steamaudio" ]]; then
    ROOT="$TMP/libs/steamaudio"
  else
    ROOT="$TMP"
  fi
else
  ROOT="$ADDON_ROOT/libs/steamaudio"
fi

pass=0
fail=0
notes=()

check() {
  local ok="$1" msg="$2"
  if [[ "$ok" == "1" ]]; then
    echo "  PASS  $msg"
    pass=$((pass + 1))
  else
    echo "  FAIL  $msg"
    fail=$((fail + 1))
    notes+=("$msg")
  fi
}

echo "==> test_package $ID (support=$SUPPORT)"
echo "    root=$ROOT"
echo "    expect lib/${OF_PATH}/${LIB_NAME}"

[[ -d "$ROOT" ]] && check 1 "root exists" || check 0 "root exists"
[[ -d "$ROOT/include" ]] && check 1 "include/ present" || check 0 "include/ present"
[[ -f "$ROOT/include/phonon.h" ]] && check 1 "phonon.h" || check 0 "phonon.h"
[[ -f "$ROOT/include/phonon_version.h" ]] && check 1 "phonon_version.h" || check 0 "phonon_version.h"

lib_dir="$ROOT/lib/${OF_PATH}"
[[ -d "$lib_dir" ]] && check 1 "lib/${OF_PATH}/ present" || check 0 "lib/${OF_PATH}/ present"

found=0
if [[ -f "${lib_dir}/${LIB_NAME}" ]]; then
  found=1
elif [[ -f "${lib_dir}/libphonon.so" ]]; then
  found=1
  LIB_NAME="libphonon.so"
elif [[ -f "${lib_dir}/libphonon.dylib" ]]; then
  found=1
  LIB_NAME="libphonon.dylib"
fi
check "$found" "primary lib (${LIB_NAME})"

if [[ "$found" == "1" ]]; then
  sz="$(wc -c < "${lib_dir}/${LIB_NAME}" | tr -d ' ')"
  if [[ "$sz" -gt 10000 ]]; then
    check 1 "lib size ok (${sz} bytes)"
  else
    check 0 "lib size too small (${sz} bytes)"
  fi
fi

# Optional deps (warn only)
for dep in libmysofa.a libpffft.a mysofa.lib pffft.lib; do
  if [[ -f "${lib_dir}/${dep}" ]]; then
    echo "  INFO  optional dep present: $dep"
  fi
done

result="pass"
[[ "$fail" -eq 0 ]] || result="fail"

echo "==> result=$result  pass=$pass fail=$fail"

if [[ -n "$STATUS_FILE" ]]; then
  # Append one JSON object line
  note_joined="$(printf '%s; ' "${notes[@]:-}" | sed 's/; $//')"
  printf '{"package_id":"%s","result":"%s","support":"%s","pass":%d,"fail":%d,"lib":"%s","notes":"%s"}\n' \
    "$ID" "$result" "$SUPPORT" "$pass" "$fail" "$LIB_NAME" "${note_joined//\"/\'}" \
    >> "$STATUS_FILE"
fi

[[ "$result" == "pass" ]]
