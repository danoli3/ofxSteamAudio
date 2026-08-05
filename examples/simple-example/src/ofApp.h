#pragma once
#include "ofMain.h"
#include "ofxSteamAudio.h"

/// Binaural HRTF demo — maps to Steam Audio itest: binauraleffect
class ofApp : public ofBaseApp {
public:
	void setup() override;
	void update() override;
	void draw() override;
	void audioOut(ofSoundBuffer& buffer) override;
	void keyPressed(int key) override;

	ofEasyCam cam;
	ofxSteamAudio::Engine audio;
	ofSoundStream soundStream;
	std::vector<int> sourceIds;
	std::vector<ofBoxPrimitive> boxes;
	bool audioOk = false;
};
