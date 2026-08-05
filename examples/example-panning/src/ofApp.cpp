#include "ofApp.h"

void ofApp::setup() {
	ofSetWindowTitle("ofxSteamAudio · Panning");
	ofSetFrameRate(60);

	audioSettings.samplingRate = 44100;
	audioSettings.frameSize = 512;
	ok = context.setup()
		&& panning.create(context, audioSettings, IPL_SPEAKERLAYOUTTYPE_STEREO)
		&& monoIn.allocate(context, 1, audioSettings.frameSize)
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
	float t = ofGetElapsedTimef();
	sourcePos = glm::vec3(3.0f * cos(t * 0.8f), 0, 3.0f * sin(t * 0.8f));
}

void ofApp::draw() {
	ofBackground(24);
	ofPushMatrix();
	ofTranslate(ofGetWidth() * 0.5f, ofGetHeight() * 0.5f);
	ofSetColor(100);
	ofDrawCircle(0, 0, 120);
	ofSetColor(255, 100, 100);
	ofDrawCircle(sourcePos.x * 40, -sourcePos.z * 40, 12);
	ofSetColor(255);
	ofDrawBitmapString("listener", -20, -10);
	ofPopMatrix();
	ofDrawBitmapStringHighlight("PanningEffect — stereo speaker pan (not HRTF)\nSource circles around listener", 20, 30);
}

void ofApp::audioOut(ofSoundBuffer& buffer) {
	if (!ok) return;
	monoIn.fillSine(330.0f, (float)audioSettings.samplingRate, 0.4f, phase);
	IPLVector3 dir = ofxSteamAudio::relativeDirection(context, sourcePos);
	panning.apply(dir, monoIn.get(), stereoOut.get());
	buffer.set(0);
	stereoOut.mixTo(buffer);
}
