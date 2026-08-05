#include "ofApp.h"

void ofApp::setup() {
	ofSetFrameRate(60);
	ofSetWindowTitle("ofxSteamAudio · Binaural (HRTF)");
	cam.setDistance(600);

	const int sampleRate = 44100;
	const int bufferSize = 512;

	audioOk = audio.setup(sampleRate, bufferSize, false);
	if (!audioOk) {
		ofLogError() << "ofxSteamAudio setup failed";
		return;
	}

	for (int i = 0; i < 4; i++) {
		ofBoxPrimitive box;
		box.set(30);
		boxes.push_back(box);
		int id = audio.addSource();
		audio.setSourceFrequency(id, 180.0f + i * 55.0f);
		audio.setSourceGain(id, 0.35f);
		sourceIds.push_back(id);
	}

	ofSoundStreamSettings settings;
	settings.numOutputChannels = 2;
	settings.numInputChannels = 0;
	settings.sampleRate = sampleRate;
	settings.bufferSize = bufferSize;
	settings.numBuffers = 4;
	settings.setOutListener(this);
	soundStream.setup(settings);
}

void ofApp::update() {
	if (!audioOk) return;
	float t = ofGetElapsedTimef();
	for (int i = 0; i < (int)boxes.size(); i++) {
		float r = 180 + i * 40;
		float a = t * (0.6f + i * 0.25f);
		glm::vec3 p(r * cos(a), 40 * sin(t * 1.5f + i), r * sin(a));
		// ofEasyCam space is larger; scale down for audio meters-ish units
		boxes[i].setPosition(p);
		audio.updateSource(sourceIds[i], p * 0.01f);
	}
	audio.setListener(cam);
}

void ofApp::draw() {
	ofBackground(20);
	cam.begin();
	ofSetColor(80, 180, 255);
	for (auto& b : boxes) b.draw();
	ofSetColor(40);
	ofDrawGrid(400, 10, false, false, false, true);
	// listener marker at camera
	ofSetColor(255, 200, 80);
	ofDrawSphere(cam.getPosition(), 8);
	cam.end();

	ofDrawBitmapStringHighlight(
		"Binaural HRTF demo (headphones recommended)\n"
		"Sources orbit listener · drag to look · keys: [space] pause stream",
		20, 30);
}

void ofApp::audioOut(ofSoundBuffer& buffer) {
	if (audioOk) audio.processAudio(buffer);
}

void ofApp::keyPressed(int key) {
	if (key == ' ') {
		// Toggle stream
		static bool running = true;
		if (running) soundStream.stop();
		else soundStream.start();
		running = !running;
	}
}
