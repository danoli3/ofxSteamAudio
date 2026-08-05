# ofxSteamAudio

**Physics-based 3D audio for openFrameworks — Steam Audio 4.8.1 (Phonon C API)**

RAII C++ wrappers covering the public Steam Audio C API, plus interactive examples that mirror Valve’s integration tests (`core/src/itest`).

![ofxSteamAudio](ofxaddons_thumbnail.png)

## Features

| Area | Wrapper types | Steam Audio itest |
|------|---------------|-------------------|
| Context / memory | `Context` | memory, log |
| Audio buffers | `AudioBuffer` | AudioBuffer |
| HRTF | `HRTF` | HRTFDatabase |
| Binaural | `BinauralEffect` | binauraleffect |
| Panning | `PanningEffect` | panningeffect |
| Virtual surround | `VirtualSurroundEffect` | virtualsurroundeffect |
| Ambisonics | Encode / Panning / Binaural / Rotation / Decode | ambisonics* |
| Direct path | `DirectEffect` + `Simulator` | directsoundeffect, directsimulator |
| Reflections | `ReflectionEffect`, `ReflectionMixer` | parametricreverb, reverbeffect, convolutioneffect |
| Pathing | `PathEffect`, probes, bakers | pathing, probes |
| Scene | `Scene`, static / instanced mesh | scene, staticmesh, instancedmesh |
| Energy / IR | `EnergyField`, `ImpulseResponse`, `Reconstructor` | energyfield, impulseresponse |
| High-level | `Engine` | multi-source binaural demo |

Also wraps: serialization, Embree device (x86_64), distance attenuation, air absorption, directivity, relative direction.

## Layout

```
src/                    # wrappers (include ofxSteamAudio.h)
libs/steamaudio/        # headers + prebuilt libphonon (+ mysofa, pffft)
examples/
  simple-example/           # Binaural HRTF multi-source
  example-panning/          # Stereo panning
  example-ambisonics/       # Ambisonics encode → binaural
  example-virtual-surround/ # 5.1 → binaural
  example-direct/           # Occlusion + direct effect
  example-scene/            # Static + instanced meshes
  example-reflections/      # Parametric reverb
  example-pathing/          # Probe generation + path scaffolding
tests/api-tests/            # Headless coverage of all wrappers
.github/workflows/macos.yml # CI: macOS build + run tests
```

## Quick start

```cpp
#include "ofxSteamAudio.h"

ofxSteamAudio::Engine audio;

void setup() {
    audio.setup(44100, 512);
    int id = audio.addSource(glm::vec3(2, 0, 0));
    audio.setSourceFrequency(id, 220.0f);

    ofSoundStreamSettings s;
    s.numOutputChannels = 2;
    s.sampleRate = 44100;
    s.bufferSize = 512;
    s.setOutListener(this);
    soundStream.setup(s);
}

void update() {
    audio.setListener(cam);
    audio.updateSource(0, sourceWorldPos);
}

void audioOut(ofSoundBuffer& buffer) {
    audio.processAudio(buffer); // synthesizes tones + binaural spatialize
}
```

Low-level API (full control):

```cpp
ofxSteamAudio::Context ctx;
ofxSteamAudio::HRTF hrtf;
ofxSteamAudio::BinauralEffect binaural;
// ctx.setup(); hrtf.create(ctx, audioSettings); binaural.create(ctx, audioSettings, hrtf);
// binaural.apply(direction, hrtf, monoIn.get(), stereoOut.get());
```

## Building

Requires openFrameworks (tested with modern OF on macOS arm64/x86_64).

### 1. Get Steam Audio binaries

Prebuilt libs are **not** committed to git. Use the library manager:

```bash
./scripts/manage_libs.sh
```

| Menu | Who | What |
|------|-----|------|
| **1 Download from release tag** | Consumers | Pull `libs-v4.8.1` assets (default install) |
| **2 Build with apothecary** | Maintainers | Standalone [apothecary](https://github.com/openframeworks/apothecary) only (no full OF) |
| **3 Package** | Maintainers | Write `scripts/dist/steamaudio-*.tar.gz` |
| **4 Upload** | Maintainers | `gh release upload` to tag `libs-v*` |
| **5 Build + package + upload** | Maintainers | Full local pipeline for one platform |

Non-interactive:

```bash
./scripts/download_libs.sh host          # this machine
./scripts/download_libs.sh osx
./scripts/build_libs.sh osx              # clones openframeworks/apothecary if needed
./scripts/package_libs.sh osx
./scripts/manage_libs.sh upload libs-v4.8.1
./scripts/manage_libs.sh list            # full platform matrix
./scripts/manage_libs.sh status
```

`build_libs.sh` does **not** need openFrameworks — only [openframeworks/apothecary](https://github.com/openframeworks/apothecary) (`APOTHECARY_DIR` / auto-cache).

Platforms (OF ∩ Steam Audio + OF extras): **osx, linux64, linuxaarch64, vs-x64, ios, android-\*, emscripten-\***, plus experimental/planned rows — see `scripts/platforms.sh`.

### 2. Build examples / tests

```bash
cd examples/simple-example && make -j
cd tests/api-tests && make -j Release && make RunRelease
```

### Link notes (macOS)

`addon_config.mk` pulls `libphonon` (+ `libmysofa` / `libpffft` when present), zlib, Accelerate, AudioToolbox, CoreAudio. Default ray tracer (no Embree) works on Apple Silicon.

## CI / releases

| Workflow | Purpose |
|----------|---------|
| [`.github/workflows/ci.yml`](.github/workflows/ci.yml) | Download libs tag → build OF → run tests/examples (macOS) |
| [`.github/workflows/build-libs.yml`](.github/workflows/build-libs.yml) | Checkout **apothecary only** (not OF) → matrix build → optional `libs-v*` upload |

**Maintainer release:**

1. Run **Build Steam Audio libs** workflow (or build locally per platform).  
2. Publish / push tag `libs-v4.8.1` with assets `steamaudio-4.8.1-<package_id>.tar.gz`.  
3. Consumers run `./scripts/download_libs.sh host`.  

## Steam Audio

- [Steam Audio GitHub](https://github.com/ValveSoftware/steam-audio)
- [C API docs](https://valvesoftware.github.io/steam-audio/doc/capi/index.html)
- Version: **4.8.1** (`STEAMAUDIO_VERSION` from `phonon_version.h`)

## License

Addon code: see `LICENSE`. Steam Audio: Apache 2.0 — see `libs/steamaudio/license/`.
