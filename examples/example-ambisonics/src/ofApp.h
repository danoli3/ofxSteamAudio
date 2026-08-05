#pragma once
#include "ofMain.h"
#include "ofxSteamAudio.h"

/// Ambisonics encode → binaural — maps to itest: ambisonicsbinauraleffect, ambisonicsrotateeffect
class ofApp : public ofBaseApp {
public:
	void setup() override;
	void update() override;
	void draw() override;
	void audioOut(ofSoundBuffer& buffer) override;

	ofxSteamAudio::Context context;
	ofxSteamAudio::HRTF hrtf;
	ofxSteamAudio::AmbisonicsEncodeEffect encode;
	ofxSteamAudio::AmbisonicsBinauralEffect ambiBin;
	ofxSteamAudio::AudioBuffer monoIn, ambi, stereoOut;
	IPLAudioSettings audioSettings{};
	ofSoundStream soundStream;
	int order = 1;
	glm::vec3 sourcePos{0, 0, -2};
	float phase = 0;
	bool ok = false;
};
