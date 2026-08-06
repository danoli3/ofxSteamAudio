#!/usr/bin/env bash
# Build Steam Audio for ofxSteamAudio via standalone openframeworks/apothecary.
# Does NOT require a full openFrameworks checkout.
#
# Usage:
#   ./scripts/build_libs.sh [package_id|host]
#
# Env:
#   APOTHECARY_DIR   Path to cloned openframeworks/apothecary (auto-cloned if missing)
#   APOTHECARY_REF   Git ref for apothecary (default: bleeding)
#   APOTHECARY       Path to apothecary executable (optional override)
#   STEAMAUDIO_VER   Version stamp (default from platforms.sh)
#   FORCE_PLANNED=1  Allow building planned platforms
#   FORCE_DOWNLOAD=1 Force apothecary re-download of sources

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ADDON_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
# shellcheck source=platforms.sh
source "$SCRIPT_DIR/platforms.sh"

APOTHECARY_REF="${APOTHECARY_REF:-bleeding}"
DEFAULT_APO_DIR="${ADDON_ROOT}/scripts/.cache/apothecary"

# Map package_id → apothecary -t TYPE and -a ARCH (host-aware for osx)
apothecary_type_arch() {
  local id="$1"
  local type arch
  type="$(platform_field "$id" apo_type)"
  arch="$(platform_field "$id" arch)"

  case "$id" in
    osx)
      # Prefer host arch; apothecary may also accept MAC / arm64 / x86_64
      if [[ "$(uname -m)" == "arm64" ]]; then arch="arm64"; else arch="x86_64"; fi
      type="osx"
      ;;
    linux64)       type="linux"; arch="64" ;;
    linuxaarch64)  type="linux"; arch="arm64" ;;
    linuxarmv7l)   type="linux"; arch="armv7" ;;
    vs-x64)        type="vs"; arch="x64" ;;
    vs-arm64)      type="vs"; arch="arm64" ;;
    vs-x86)        type="vs"; arch="Win32" ;;
    msys2)         type="msys2"; arch="64" ;;
    ios)           type="ios"; arch="arm64" ;;
    ios-sim)       type="ios"; arch="x86_64" ;;
    tvos)          type="tvos"; arch="arm64" ;;
    android-arm64) type="android"; arch="arm64" ;;
    android-armv7) type="android"; arch="armv7" ;;
    android-x86)   type="android"; arch="x86" ;;
    android-x86_64) type="android"; arch="x86_64" ;;
    emscripten-wasm)   type="emscripten"; arch="64" ;;
    emscripten-wasm64) type="emscripten"; arch="64" ;; # wasm64 if supported by formula/TYPE
    watchos)       type="watchos"; arch="arm64" ;;
    xros)          type="xros"; arch="arm64" ;;
    catos)         type="catos"; arch="arm64" ;;
  esac
  echo "${type}|${arch}"
}

ensure_apothecary() {
  if [[ -n "${APOTHECARY:-}" && -f "$APOTHECARY" ]]; then
    APOTHECARY_DIR="$(cd "$(dirname "$APOTHECARY")/.." && pwd)"
    # If user pointed at .../apothecary/apothecary (binary), parent is apo root when structure is apo/apothecary/apothecary
    if [[ -d "$(dirname "$APOTHECARY")/formulas" ]]; then
      APOTHECARY_DIR="$(cd "$(dirname "$APOTHECARY")" && pwd)"
    elif [[ -d "$(dirname "$APOTHECARY")/../formulas" ]]; then
      APOTHECARY_DIR="$(cd "$(dirname "$APOTHECARY")/.." && pwd)"
    fi
    echo "$APOTHECARY_DIR"
    return
  fi

  if [[ -n "${APOTHECARY_DIR:-}" && -d "$APOTHECARY_DIR/apothecary" ]]; then
    echo "$APOTHECARY_DIR"
    return
  fi

  # Prefer OF-bundled apothecary only if already present (optional convenience)
  local cand
  for cand in \
    "$ADDON_ROOT/../../scripts/apothecary" \
    "$ADDON_ROOT/../../../scripts/apothecary"
  do
    if [[ -d "$cand/apothecary" && -f "$cand/apothecary/apothecary" ]]; then
      echo "$(cd "$cand" && pwd)"
      return
    fi
  done

  # Clone standalone openframeworks/apothecary
  if [[ ! -d "$DEFAULT_APO_DIR/.git" ]]; then
    echo "==> Cloning openframeworks/apothecary (${APOTHECARY_REF}) → $DEFAULT_APO_DIR"
    mkdir -p "$(dirname "$DEFAULT_APO_DIR")"
    git clone --depth 1 --branch "$APOTHECARY_REF" \
      https://github.com/openframeworks/apothecary.git "$DEFAULT_APO_DIR" \
      || git clone --depth 1 https://github.com/openframeworks/apothecary.git "$DEFAULT_APO_DIR"
  else
    echo "==> Using existing apothecary at $DEFAULT_APO_DIR"
    git -C "$DEFAULT_APO_DIR" fetch --depth 1 origin "$APOTHECARY_REF" 2>/dev/null || true
    git -C "$DEFAULT_APO_DIR" checkout "$APOTHECARY_REF" 2>/dev/null || true
  fi
  echo "$DEFAULT_APO_DIR"
}

find_apothecary_bin() {
  local root="$1"
  local c
  for c in \
    "$root/apothecary/apothecary" \
    "$root/apothecary" \
    "$root/apothecary.sh"
  do
    if [[ -f "$c" ]]; then
      echo "$c"
      return
    fi
  done
  return 1
}

install_formula() {
  local apo_root="$1"
  local formula_src="${SCRIPT_DIR}/apothecary/formula/steamaudio.sh"
  local dest=""

  if [[ -d "$apo_root/apothecary/formulas" ]]; then
    dest="$apo_root/apothecary/formulas/steamaudio.sh"
  elif [[ -d "$apo_root/formulas" ]]; then
    dest="$apo_root/formulas/steamaudio.sh"
  else
    mkdir -p "$apo_root/apothecary/formulas"
    dest="$apo_root/apothecary/formulas/steamaudio.sh"
  fi

  cp -f "$formula_src" "$dest"
  # Also keep addon-local formulas path for documentation / alternate discovery
  mkdir -p "$ADDON_ROOT/scripts/formulas"
  cp -f "$formula_src" "$ADDON_ROOT/scripts/formulas/steamaudio.sh"
  echo "[ok] formula → $dest"
}

run_platform_install_script() {
  local apo_root="$1"
  local type="$2"
  # Optional environment bootstrap from apothecary repo
  local script=""
  case "$type" in
    osx|ios|tvos|watchos|xros|catos)
      script="$apo_root/scripts/osx/install.sh"
      ;;
    linux)
      script="$apo_root/scripts/linux/install.sh"
      [[ -f "$script" ]] || script="$apo_root/scripts/linux64/install.sh"
      ;;
    vs)
      script="$apo_root/scripts/vs/install.sh"
      ;;
    android)
      script="$apo_root/scripts/android/install.sh"
      ;;
    emscripten)
      script="$apo_root/scripts/emscripten/install.sh"
      ;;
    msys2)
      script="$apo_root/scripts/msys2/install.sh"
      ;;
  esac
  if [[ -n "$script" && -f "$script" ]]; then
    echo "==> Running apothecary install: $script"
    bash "$script" || echo "warning: install script returned non-zero (continuing)"
  fi
}

copy_build_into_addon() {
  local id="$1"
  local out_root="$2"   # CUSTOM_LIBS_DIR passed to apothecary
  local of_path apo_type
  of_path="$(platform_field "$id" of_path)"
  apo_type="$(platform_field "$id" apo_type)"

  # Apothecary installs as out_root/steamaudio/... or out_root/...
  local candidates=(
    "$out_root/steamaudio"
    "$out_root"
    "${out_root}/../steamaudio"
  )
  local src="" c
  for c in "${candidates[@]}"; do
    if [[ -d "$c/include" ]] || [[ -d "$c/lib" ]]; then
      src="$c"
      break
    fi
  done
  if [[ -z "$src" ]]; then
    echo "warning: searching for phonon under $out_root ..." >&2
    local hit
    hit="$(find "$out_root" -name 'libphonon.a' -o -name 'phonon.lib' -o -name 'libphonon.so' 2>/dev/null | head -1 || true)"
    if [[ -n "$hit" ]]; then
      src="$(cd "$(dirname "$hit")/../.." && pwd)"
      # if hit is .../lib/osx/libphonon.a, parent of lib is steamaudio root
      if [[ -d "$(dirname "$hit")/.." ]]; then
        local maybe
        maybe="$(cd "$(dirname "$hit")/.." && pwd)"
        [[ -d "$maybe" ]] && src="$maybe"
      fi
    fi
  fi

  if [[ -z "$src" || ! -d "$src" ]]; then
    echo "error: could not locate apothecary output for steamaudio under $out_root" >&2
    find "$out_root" -maxdepth 5 -type f 2>/dev/null | head -40 >&2 || true
    return 1
  fi

  local dest="${ADDON_ROOT}/libs/steamaudio"
  mkdir -p "$dest/lib/${of_path}" "$dest/include" "$dest/license"

  if [[ -d "$src/include" ]]; then
    cp -R "$src/include/." "$dest/include/"
  fi
  if [[ -d "$src/license" ]]; then
    cp -R "$src/license/." "$dest/license/"
  fi

  local lib_src=""
  for c in \
    "$src/lib/${of_path}" \
    "$src/lib/${apo_type}" \
    "$src/lib/osx" \
    "$src/lib/linux" \
    "$src/lib/vs" \
    "$src/lib"
  do
    if [[ -d "$c" ]] && ls "$c"/*phonon* >/dev/null 2>&1; then
      lib_src="$c"
      break
    fi
    # nested arch folders
    if [[ -d "$c" ]]; then
      local nested
      nested="$(find "$c" -maxdepth 2 -name '*phonon*' -type f 2>/dev/null | head -1 || true)"
      if [[ -n "$nested" ]]; then
        lib_src="$(dirname "$nested")"
        break
      fi
    fi
  done

  if [[ -z "$lib_src" ]]; then
    echo "error: no phonon library under $src" >&2
    find "$src" -name '*phonon*' 2>/dev/null | head -20 >&2 || true
    return 1
  fi

  mkdir -p "$dest/lib/${of_path}"
  cp -R "$lib_src/." "$dest/lib/${of_path}/"

  for dep in libmysofa.a libpffft.a mysofa.lib pffft.lib libmysofa.so libpffft.so; do
    if [[ -f "$lib_src/$dep" ]]; then
      cp -f "$lib_src/$dep" "$dest/lib/${of_path}/"
    fi
  done

  echo "${STEAMAUDIO_VER}" > "$dest/VERSION"
  echo "apothecary:${id}" >> "$dest/INSTALLED_PLATFORMS"
  sort -u "$dest/INSTALLED_PLATFORMS" -o "$dest/INSTALLED_PLATFORMS" 2>/dev/null || true
  echo "[ok] installed → libs/steamaudio/lib/${of_path}"
}

build_one() {
  local id="$1"
  local support type arch pair apo_root apo_bin out_dir
  if ! platform_field "$id" id >/dev/null; then
    echo "error: unknown package_id '$id'" >&2
    platform_ids | sed 's/^/  /' >&2
    return 1
  fi
  support="$(platform_field "$id" support)"
  if [[ "$support" == "planned" && "${FORCE_PLANNED:-0}" != "1" ]]; then
    echo "error: $id is planned (set FORCE_PLANNED=1 to try)" >&2
    return 1
  fi

  pair="$(apothecary_type_arch "$id")"
  type="${pair%%|*}"
  arch="${pair##*|}"

  apo_root="$(ensure_apothecary)"
  apo_bin="$(find_apothecary_bin "$apo_root")" || {
    echo "error: apothecary binary not found under $apo_root" >&2
    return 1
  }
  echo "APOTHECARY_DIR=$apo_root"
  echo "APOTHECARY=$apo_bin"
  echo "TYPE=$type ARCH=$arch package_id=$id"

  install_formula "$apo_root"
  run_platform_install_script "$apo_root" "$type"

  out_dir="${ADDON_ROOT}/scripts/.cache/apo-out/${id}"
  rm -rf "$out_dir"
  mkdir -p "$out_dir"

  local apo_args=( -t "$type" -a "$arch" -d "$out_dir" -j "${PARALLEL_MAKE:-$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 4)}" )
  if [[ "${FORCE_DOWNLOAD:-0}" == "1" ]]; then
    apo_args+=( -f )
  fi
  if [[ "${A_VERBOSE:-0}" == "1" ]]; then
    apo_args+=( -v )
  fi

  echo "==> apothecary ${apo_args[*]} update steamaudio"
  # Run from apothecary root so relative formula paths resolve
  pushd "$apo_root" >/dev/null
  # Binary may live in apothecary/ subdir
  if [[ -x "$apo_bin" ]]; then
    "$apo_bin" "${apo_args[@]}" update steamaudio
  else
    bash "$apo_bin" "${apo_args[@]}" update steamaudio
  fi
  popd >/dev/null

  copy_build_into_addon "$id" "$out_dir"
}

main() {
  local target="${1:-host}"
  case "$target" in
    -h|--help)
      cat <<EOF
build_libs.sh — build Steam Audio with standalone apothecary (no openFrameworks)

Usage:
  $0 [host|<package_id>]

Env:
  APOTHECARY_DIR   Clone of github.com/openframeworks/apothecary
  APOTHECARY_REF   Branch/tag (default: bleeding)
  APOTHECARY       Path to apothecary script
  FORCE_DOWNLOAD=1 FORCE_PLANNED=1 A_VERBOSE=1 PARALLEL_MAKE=N

Examples:
  $0 osx
  APOTHECARY_DIR=~/apothecary $0 linux64
EOF
      exit 0
      ;;
    host) build_one "$(detect_host_package_id)" ;;
    *) build_one "$target" ;;
  esac
}

main "$@"
