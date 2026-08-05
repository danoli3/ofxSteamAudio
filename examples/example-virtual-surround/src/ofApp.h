#pragma once
#include "ofMain.h"
#include "ofxSteamAudio.h"

/// 5.1 virtual surround → binaural — maps to itest: virtualsurroundeffect
class ofApp : public ofBaseApp {
public:
	void setup() override;
	void update() override;
	void draw() override;
	void audioOut(ofSoundBuffer& buffer) override;

	ofxSteamAudio::Context context;
	ofxSteamAudio::HRTF hrtf;
	ofxSteamAudio::VirtualSurroundEffect virtualSurround;
	ofxSteamAudio::AudioBuffer surroundIn, stereoOut;
	IPLAudioSettings audioSettings{};
	ofSoundStream soundStream;
	float phase = 0;
	int activeChannel = 0;
	bool ok = false;
};
