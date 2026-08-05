#pragma once
#include "ofMain.h"
#include "ofxSteamAudio.h"

/// Parametric reflection / reverb — maps to itest: parametricreverb, reverbeffect
class ofApp : public ofBaseApp {
public:
	void setup() override;
	void update() override;
	void draw() override;
	void audioOut(ofSoundBuffer& buffer) override;
	void keyPressed(int key) override;

	ofxSteamAudio::Context context;
	ofxSteamAudio::HRTF hrtf;
	ofxSteamAudio::BinauralEffect binaural;
	ofxSteamAudio::ReflectionEffect reflection;
	ofxSteamAudio::AudioBuffer monoIn, dryStereo, wetMono, wetStereo, mix;
	IPLAudioSettings audioSettings{};
	ofSoundStream soundStream;
	glm::vec3 sourcePos{1, 0, -2};
	float phase = 0;
	float reverbTime = 1.2f;
	float wetDry = 0.35f;
	bool ok = false;
};
