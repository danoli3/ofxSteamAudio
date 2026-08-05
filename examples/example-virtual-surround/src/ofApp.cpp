#include "ofApp.h"

// 5.1 channel order (Steam Audio / typical): FL, FR, C, LFE, RL, RR
static const char* kChNames[] = { "FL", "FR", "C", "LFE", "RL", "RR" };

void ofApp::setup() {
	ofSetWindowTitle("ofxSteamAudio · Virtual Surround");
	ofSetFrameRate(60);

	audioSettings.samplingRate = 44100;
	audioSettings.frameSize = 512;

	ok = context.setup()
		&& hrtf.create(context, audioSettings)
		&& virtualSurround.create(context, audioSettings, hrtf, IPL_SPEAKERLAYOUTTYPE_SURROUND_5_1)
		&& surroundIn.allocate(context, 6, audioSettings.frameSize)
		&& stereoOut.allocate(context, 2, audioSettings.frameSize);
	if (!ok) return;

	ofSoundStreamSettings s;
	s.numOutputChannels = 2;
	s.sampleRate = audioSettings.samplingRate;
	s.bufferSize = audioSettings.frameSize;
	s.setOutListener(this);
	soundStream.setup(s);
}

void ofApp::update() {
	// Cycle which speaker is "active" every 1.5s
	activeChannel = ((int)(ofGetElapsedTimef() / 1.5f)) % 6;
}

void ofApp::draw() {
	ofBackground(20);
	ofDrawBitmapStringHighlight(
		"VirtualSurroundEffect — 5.1 bed → binaural\n"
		"Active speaker: " + string(kChNames[activeChannel]) + " (" + ofToString(activeChannel) + ")\n"
		"Headphones recommended", 20, 30);

	// Simple speaker layout visualization
	ofPushMatrix();
	ofTranslate(ofGetWidth() * 0.5f, ofGetHeight() * 0.55f);
	glm::vec2 pos[6] = {
		{-80, -60}, {80, -60}, {0, -80}, {0, 20}, {-100, 80}, {100, 80}
	};
	for (int i = 0; i < 6; i++) {
		ofSetColor(i == activeChannel ? ofColor(255, 180, 60) : ofColor(80));
		ofDrawCircle(pos[i], 18);
		ofSetColor(255);
		ofDrawBitmapString(kChNames[i], pos[i].x - 10, pos[i].y + 4);
	}
	ofPopMatrix();
}

void ofApp::audioOut(ofSoundBuffer& buffer) {
	if (!ok) return;
	surroundIn.clear();
	// Fill only the active channel with a tone
	float amp = 0.45f;
	const float inc = 440.0f * TWO_PI / audioSettings.samplingRate;
	for (int i = 0; i < audioSettings.frameSize; ++i) {
		phase += inc;
		if (phase > TWO_PI) phase -= TWO_PI;
		surroundIn.channel(activeChannel)[i] = sinf(phase) * amp;
	}
	virtualSurround.apply(hrtf, surroundIn.get(), stereoOut.get());
	buffer.set(0);
	stereoOut.mixTo(buffer);
}
