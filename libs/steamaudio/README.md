# Steam Audio prebuilt libraries

Headers under `include/` and licenses may ship with the repo. **Binary libraries** under `lib/` are **not** stored in git.

## Install (consumers)

From the addon root:

```bash
./scripts/manage_libs.sh              # menu → "Download from release tag"
# or non-interactive:
./scripts/download_libs.sh host       # this machine's platform
./scripts/download_libs.sh osx
./scripts/download_libs.sh all        # all full/experimental assets
```

Release tag (default): `libs-v4.8.1`  
Override: `LIBS_TAG=libs-v4.8.1 REPO=owner/ofxSteamAudio ./scripts/download_libs.sh osx`

## Build & publish (maintainers)

```bash
./scripts/manage_libs.sh
# 2) Build with apothecary
# 3) Package
# 4) Upload to GitHub release
# or full pipeline: 5)
```

CI: `.github/workflows/build-libs.yml` builds a matrix and can attach assets to tag `libs-v*`.

## Platform matrix

```bash
./scripts/manage_libs.sh list
```

| package_id | OF `lib/` path | Steam Audio / OF |
|------------|----------------|------------------|
| osx | osx | macOS |
| linux64 | linux64 | Linux x64 |
| linuxaarch64 | linuxaarch64 | Linux ARM64 |
| vs-x64 | vs/x64 | Windows MSVC |
| ios | ios | iOS |
| android-arm64 | android/arm64 | Android |
| emscripten-wasm | emscripten/WASM | Web |
| … | … | see `scripts/platforms.sh` |

Asset name: `steamaudio-4.8.1-<package_id>.tar.gz`
