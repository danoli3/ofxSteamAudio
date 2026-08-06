# ofxSteamAudio scripts

## Library lifecycle

```
  openframeworks/apothecary only (NOT full openFrameworks)
                    ┌─────────────────────┐
   clone/cache      │  build_libs.sh      │
   apothecary +     │  manage_libs → 2/5  │
   formula          └──────────┬──────────┘
                               │ libs/steamaudio/lib/<of_path>/
                               v
                    ┌─────────────────────┐
                    │  package_libs.sh    │
                    │  manage_libs → 3    │
                    └──────────┬──────────┘
                               │ scripts/dist/steamaudio-VER-id.tar.gz
                               v
                    ┌─────────────────────┐
   gh release       │  manage_libs upload │
   tag libs-v*      │  or GHA publish     │
                    └──────────┬──────────┘
                               │
                               v
                    ┌─────────────────────┐
   consumers / CI   │  download_libs.sh   │
                    │  manage_libs → 1    │
                    └─────────────────────┘
```

`build_libs.sh` sets `APOTHECARY_DIR` to a clone of
[openframeworks/apothecary](https://github.com/openframeworks/apothecary)
(under `scripts/.cache/apothecary` if unset), installs
`scripts/apothecary/formula/steamaudio.sh`, and runs:

```bash
./apothecary -t <type> -a <arch> -d <out> update steamaudio
```

## Which workflow uses what

| GitHub Action | Checkouts | Purpose |
|---------------|-----------|---------|
| `ci.yml` | **ofxSteamAudio only** | Download release libs + verify |
| `ci.yml` optional OF job | OF + addon | Examples/tests (manual dispatch) |
| `build-libs.yml` | **apothecary + addon only** | Compile phonon with apothecary |

## Entry point

```bash
./scripts/manage_libs.sh          # menu
./scripts/manage_libs.sh --help
```

## Files

| Script | Role |
|--------|------|
| `platforms.sh` | Shared matrix (OF path ↔ apothecary type ↔ support level) |
| `manage_libs.sh` | Menu + upload/status/clean |
| `download_libs.sh` | Fetch release assets |
| `build_libs.sh` | Apothecary build → install into addon |
| `package_libs.sh` | tar.gz for GitHub Releases |
| `apothecary/formula/steamaudio.sh` | Apothecary formula (Steam Audio 4.8.1) |

## Environment

| Variable | Default | Meaning |
|----------|---------|---------|
| `STEAMAUDIO_VER` | `4.8.1` | Phonon version |
| `LIBS_TAG` | `libs-v4.8.1` | GitHub release tag for binaries |
| `REPO` | git origin | `owner/name` for download/upload |
| `APOTHECARY_DIR` | `scripts/.cache/apothecary` | Standalone apothecary clone |
| `APOTHECARY_REF` | `bleeding` | apothecary git branch/tag |
| `APOTHECARY` | auto | path to apothecary script |
| `FORCE=1` | | Redownload even if present |
| `FORCE_DOWNLOAD=1` | | Force apothecary source re-fetch |
| `FORCE_PLANNED=1` | | Allow building “planned” platforms |

## Asset naming

```text
steamaudio-4.8.1-osx.tar.gz
steamaudio-4.8.1-linux64.tar.gz
steamaudio-4.8.1-vs-x64.tar.gz
steamaudio-4.8.1-ios.tar.gz
steamaudio-4.8.1-android-arm64.tar.gz
steamaudio-4.8.1-emscripten-wasm.tar.gz
...
```

Each archive contains:

```text
steamaudio/
  include/
  license/
  lib/<of_path>/libphonon.*
  PACKAGE.txt
```
