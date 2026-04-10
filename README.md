# ofxSteamAudio

**Physics-Based 3D Audio • Powered by Steam Audio 4.8.1**

High-performance spatial audio, occlusion, reflections, pathing, and baking for openFrameworks.

![ofxSteamAudio](ofxaddons_thumbnail.png)

### Features
- Full Steam Audio 4.8.1 API access
- Real-time HRTF binaural rendering
- Dynamic & baked reflections (reverb)
- Occlusion & transmission
- Sound propagation / pathing
- Probe baking system
- Multi-platform (Windows, macOS, Linux)

### Quick Start
```cpp
ofxSteamAudioContext context;
ofxSteamAudioScene scene(context);
ofxSteamAudioSource source(context, scene);

source.setPosition(ofVec3f(10, 0, 0));

# Steam Audio 4.8.1

## Additional Links

Valve Corporation
- [Steam Audio GitHub](https://github.com/valvesoftware/steam-audio)
- [Steam Audio Homepage](https://valvesoftware.github.io/steam-audio)
- [Downloads & Documentation](https://valvesoftware.github.io/steam-audio/downloads.html)
- [Community Forum](http://steamcommunity.com/app/596420/discussions/)
- [News & Updates](http://steamcommunity.com/app/596420/allnews/)
