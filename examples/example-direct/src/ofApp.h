#pragma once
#include "ofMain.h"
#include "ofxSteamAudio.h"

/// Distance / air / occlusion + binaural — maps to itest: directsoundeffect, directsimulator
class ofApp : public ofBaseApp {
public:
	void setup() override;
	void update() override;
	void draw() override;
	void audioOut(ofSoundBuffer& buffer) override;

	ofxSteamAudio::Context context;
	ofxSteamAudio::HRTF hrtf;
	ofxSteamAudio::BinauralEffect binaural;
	ofxSteamAudio::DirectEffect direct;
	ofxSteamAudio::Scene scene;
	ofxSteamAudio::Simulator simulator;
	ofxSteamAudio::SimSource simSource;
	ofxSteamAudio::AudioBuffer monoIn, directOut, stereoOut;
	IPLAudioSettings audioSettings{};
	ofSoundStream soundStream;
	glm::vec3 sourcePos{0, 1.5f, -8};
	glm::vec3 listenerPos{0, 1.6f, 0};
	float phase = 0;
	float lastOcclusion = 0;
	float lastAttenuation = 1;
	bool ok = false;
};
