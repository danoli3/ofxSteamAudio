#pragma once
#include "ofMain.h"
#include "ofxSteamAudio.h"

/// Speaker panning demo — maps to Steam Audio itest: panningeffect
class ofApp : public ofBaseApp {
public:
	void setup() override;
	void update() override;
	void draw() override;
	void audioOut(ofSoundBuffer& buffer) override;

	ofxSteamAudio::Context context;
	ofxSteamAudio::PanningEffect panning;
	ofxSteamAudio::AudioBuffer monoIn, stereoOut;
	IPLAudioSettings audioSettings{};
	ofSoundStream soundStream;
	glm::vec3 sourcePos{2, 0, 0};
	float phase = 0;
	bool ok = false;
};
