#!/usr/bin/env bash
# Interactive manager for ofxSteamAudio prebuilt binaries.
#
# Default maintainer flow:  build (apothecary) → package → upload release tag
# Default consumer flow:    download from release tag
#
# Usage:
#   ./scripts/manage_libs.sh              # interactive menu
#   ./scripts/manage_libs.sh download [host|all|id]
#   ./scripts/manage_libs.sh build [host|id]
#   ./scripts/manage_libs.sh package [host|all|id]
#   ./scripts/manage_libs.sh upload [tag]
#   ./scripts/manage_libs.sh status
#   ./scripts/manage_libs.sh list
#   ./scripts/manage_libs.sh clean

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ADDON_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
# shellcheck source=platforms.sh
source "$SCRIPT_DIR/platforms.sh"

chmod +x "$SCRIPT_DIR/download_libs.sh" "$SCRIPT_DIR/package_libs.sh" "$SCRIPT_DIR/build_libs.sh" 2>/dev/null || true

detect_repo() {
  if [[ -n "${REPO:-}" ]]; then echo "$REPO"; return; fi
  local url
  url="$(git -C "$ADDON_ROOT" remote get-url origin 2>/dev/null || true)"
  if [[ "$url" =~ github.com[:/]([^/]+)/([^/.]+) ]]; then
    echo "${BASH_REMATCH[1]}/${BASH_REMATCH[2]}"
    return
  fi
  echo "(set REPO=owner/name)"
}

status_report() {
  echo "=== ofxSteamAudio binary status ==="
  echo "Steam Audio version:  $STEAMAUDIO_VER"
  echo "Release tag (libs):   $LIBS_TAG"
  echo "Repo:                 $(detect_repo)"
  echo "Host package id:      $(detect_host_package_id)"
  echo "Addon root:           $ADDON_ROOT"
  echo
  if [[ -f "$ADDON_ROOT/libs/steamaudio/VERSION" ]]; then
    echo "Installed VERSION:    $(cat "$ADDON_ROOT/libs/steamaudio/VERSION")"
  else
    echo "Installed VERSION:    (none)"
  fi
  if [[ -f "$ADDON_ROOT/libs/steamaudio/LIBS_TAG" ]]; then
    echo "Installed LIBS_TAG:   $(cat "$ADDON_ROOT/libs/steamaudio/LIBS_TAG")"
  fi
  echo
  printf "%-20s %-12s %-8s %s\n" "PACKAGE_ID" "SUPPORT" "STATUS" "OF PATH"
  printf "%-20s %-12s %-8s %s\n" "----------" "-------" "------" "-------"
  local id of_path sup libdir status
  for id in $(platform_ids); do
    of_path="$(platform_field "$id" of_path)"
    sup="$(platform_field "$id" support)"
    libdir="$ADDON_ROOT/libs/steamaudio/lib/${of_path}"
    if [[ -d "$libdir" ]] && ls "$libdir"/*phonon* >/dev/null 2>&1; then
      status="yes"
    else
      status="-"
    fi
    printf "%-20s %-12s %-8s %s\n" "$id" "$sup" "$status" "lib/${of_path}"
  done
  echo
  if [[ -d "$SCRIPT_DIR/dist" ]]; then
    echo "Packaged assets (scripts/dist):"
    ls -lh "$SCRIPT_DIR/dist"/*.tar.gz 2>/dev/null || echo "  (none)"
  fi
}

upload_release() {
  local tag="${1:-$LIBS_TAG}"
  local dist="$SCRIPT_DIR/dist"
  if ! command -v gh >/dev/null 2>&1; then
    echo "error: GitHub CLI 'gh' required for upload" >&2
    echo "  brew install gh && gh auth login" >&2
    echo "  Or upload scripts/dist/*.tar.gz manually to release $tag" >&2
    return 1
  fi
  if ! ls "$dist"/*.tar.gz >/dev/null 2>&1; then
    echo "error: no packages in scripts/dist — run package first" >&2
    return 1
  fi

  echo "==> Ensuring release $tag exists"
  if ! gh release view "$tag" --repo "$(detect_repo)" >/dev/null 2>&1; then
    gh release create "$tag" \
      --repo "$(detect_repo)" \
      --title "Steam Audio ${STEAMAUDIO_VER} prebuilt libs" \
      --notes "Prebuilt Steam Audio ${STEAMAUDIO_VER} binaries for ofxSteamAudio (apothecary).

Install:
\`\`\`bash
./scripts/download_libs.sh host    # or: all | osx | linux64 | vs-x64 | ...
\`\`\`

Built with scripts/apothecary/formula/steamaudio.sh (BUILD_ID=${BUILD_ID}).
"
  fi

  echo "==> Uploading assets to $tag"
  gh release upload "$tag" "$dist"/*.tar.gz --repo "$(detect_repo)" --clobber
  echo "[ok] uploaded to https://github.com/$(detect_repo)/releases/tag/${tag}"
}

clean_libs() {
  echo "This removes downloaded/built binaries under libs/steamaudio/lib"
  echo "Headers/license are kept if present."
  rm -rf "$ADDON_ROOT/libs/steamaudio/lib"
  rm -f "$ADDON_ROOT/libs/steamaudio/INSTALLED_PLATFORMS" \
        "$ADDON_ROOT/libs/steamaudio/LIBS_TAG"
  rm -rf "$SCRIPT_DIR/.cache"
  echo "[ok] cleaned lib/ and scripts/.cache"
}

pick_platform() {
  local ids=() id
  while IFS= read -r id; do ids+=("$id"); done < <(platform_ids)
  ids+=("host" "all" "cancel")
  echo "Select platform package id:" >&2
  local i=1
  for id in "${ids[@]}"; do
    if [[ "$id" == "host" || "$id" == "all" || "$id" == "cancel" ]]; then
      printf "  %2d) %s\n" "$i" "$id" >&2
    else
      printf "  %2d) %-20s [%s] %s\n" "$i" "$id" \
        "$(platform_field "$id" support)" "$(platform_field "$id" desc)" >&2
    fi
    i=$((i + 1))
  done
  local choice
  read -r -p "Number [host]: " choice
  choice="${choice:-}"
  if [[ -z "$choice" ]]; then
    echo "host"
    return
  fi
  if [[ "$choice" =~ ^[0-9]+$ ]] && (( choice >= 1 && choice <= ${#ids[@]} )); then
    echo "${ids[$((choice - 1))]}"
    return
  fi
  echo "$choice"
}

menu() {
  cat <<EOF

 ofxSteamAudio · library manager
 Steam Audio ${STEAMAUDIO_VER}  ·  release tag ${LIBS_TAG}
 repo $(detect_repo)

  1) Download from release tag          (consumer default)
  2) Build with apothecary only         (no openFrameworks; maintainer default)
  3) Package libs for release           → scripts/dist/*.tar.gz
  4) Upload packages to GitHub release  (needs gh)
  5) Build + package + upload           (full maintainer pipeline)
  6) Status / verify installed platforms
  7) List all platforms (+ CI runners)
  8) Clean local lib binaries
  9) Download ALL full/experimental platforms
  s) Full platform report (local + release probe → PLATFORM_STATUS.md)
  t) Test installed package (pick platform)
  0) Quit

EOF
  local opt plat
  read -r -p "Choice [1]: " opt
  opt="${opt:-1}"
  case "$opt" in
    1)
      plat="$(pick_platform)"
      [[ "$plat" == "cancel" ]] && return 0
      "$SCRIPT_DIR/download_libs.sh" "$plat"
      ;;
    2)
      plat="$(pick_platform)"
      [[ "$plat" == "cancel" || "$plat" == "all" ]] && {
        [[ "$plat" == "all" ]] && echo "Build one platform at a time (CI matrix builds all)." >&2
        return 0
      }
      "$SCRIPT_DIR/build_libs.sh" "$plat"
      ;;
    3)
      plat="$(pick_platform)"
      [[ "$plat" == "cancel" ]] && return 0
      "$SCRIPT_DIR/package_libs.sh" "$plat"
      ;;
    4)
      read -r -p "Release tag [$LIBS_TAG]: " tag
      tag="${tag:-$LIBS_TAG}"
      upload_release "$tag"
      ;;
    5)
      plat="$(pick_platform)"
      [[ "$plat" == "cancel" || "$plat" == "all" ]] && {
        echo "Pick a single platform for local full pipeline (use GHA for all)." >&2
        return 0
      }
      "$SCRIPT_DIR/build_libs.sh" "$plat"
      "$SCRIPT_DIR/package_libs.sh" "$plat"
      read -r -p "Upload to $LIBS_TAG now? [y/N]: " yn
      if [[ "$yn" =~ ^[Yy] ]]; then
        upload_release "$LIBS_TAG"
      else
        echo "Skipped upload. Packages in scripts/dist/"
      fi
      ;;
    6) status_report ;;
    7) list_platforms_table ;;
    8)
      read -r -p "Delete libs/steamaudio/lib? [y/N]: " yn
      [[ "$yn" =~ ^[Yy] ]] && clean_libs
      ;;
    9) "$SCRIPT_DIR/download_libs.sh" all ;;
    s|S)
      "$SCRIPT_DIR/report_status.sh" --from-local --from-release --write "$ADDON_ROOT/PLATFORM_STATUS.md"
      echo "See PLATFORM_STATUS.md"
      ;;
    t|T)
      plat="$(pick_platform)"
      [[ "$plat" == "cancel" || "$plat" == "all" ]] && return 0
      "$SCRIPT_DIR/test_package.sh" "$plat"
      ;;
    0|q|Q) exit 0 ;;
    *) echo "Unknown option: $opt" >&2 ;;
  esac
}

main() {
  local cmd="${1:-}"
  if [[ -z "$cmd" ]]; then
    menu
    exit 0
  fi
  shift || true
  case "$cmd" in
    download) "$SCRIPT_DIR/download_libs.sh" "${1:-host}" ;;
    build)    "$SCRIPT_DIR/build_libs.sh" "${1:-host}" ;;
    package)  "$SCRIPT_DIR/package_libs.sh" "${1:-host}" ;;
    upload)   upload_release "${1:-$LIBS_TAG}" ;;
    status)   status_report; "$SCRIPT_DIR/report_status.sh" --from-local --from-release --write "$ADDON_ROOT/PLATFORM_STATUS.md" || true ;;
    list)     list_platforms_table ;;
    report)   "$SCRIPT_DIR/report_status.sh" --from-local --from-release --write "$ADDON_ROOT/PLATFORM_STATUS.md" ;;
    test)     "$SCRIPT_DIR/test_package.sh" "${1:-host}" ;;
    clean)    clean_libs ;;
    menu)     menu ;;
    -h|--help|help)
      cat <<EOF
manage_libs.sh — ofxSteamAudio binary lifecycle

Interactive:
  $0

Commands:
  download [host|all|id]   Fetch from GitHub release ${LIBS_TAG}
  build [host|id]          Apothecary build into libs/steamaudio
  package [host|all|id]    Create scripts/dist/steamaudio-*.tar.gz
  upload [tag]             gh release upload packages
  status                   Installed platforms + PLATFORM_STATUS.md report
  report                   Full local + release probe report
  test [id]                Verify package layout (no OF)
  list                     Platform matrix (+ CI runners)
  clean                    Remove local lib binaries

Env:
  STEAMAUDIO_VER  LIBS_TAG  REPO  APOTHECARY_DIR  FORCE=1

Default release tag: ${LIBS_TAG}
EOF
      ;;
    *)
      echo "Unknown command: $cmd (try --help)" >&2
      exit 1
      ;;
  esac
}

main "$@"
