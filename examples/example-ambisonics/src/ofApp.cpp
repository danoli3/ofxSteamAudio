#include "ofApp.h"

void ofApp::setup() {
	ofSetWindowTitle("ofxSteamAudio · Ambisonics");
	ofSetFrameRate(60);

	audioSettings.samplingRate = 44100;
	audioSettings.frameSize = 512;
	order = 1;
	const int nCh = ofxSteamAudio::ambisonicsChannels(order);

	ok = context.setup()
		&& hrtf.create(context, audioSettings)
		&& encode.create(context, audioSettings, order)
		&& ambiBin.create(context, audioSettings, hrtf, order)
		&& monoIn.allocate(context, 1, audioSettings.frameSize)
		&& ambi.allocate(context, nCh, audioSettings.frameSize)
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
	sourcePos = glm::vec3(2.5f * cos(t * 0.7f), 0.5f * sin(t), 2.5f * sin(t * 0.7f));
}

void ofApp::draw() {
	ofBackground(18);
	ofPushMatrix();
	ofTranslate(ofGetWidth() * 0.5f, ofGetHeight() * 0.5f);
	ofNoFill();
	ofSetColor(80);
	ofDrawCircle(0, 0, 100);
	ofFill();
	ofSetColor(120, 220, 160);
	ofDrawSphere(sourcePos.x * 35, -sourcePos.y * 35, 0, 10);
	ofPopMatrix();
	ofDrawBitmapStringHighlight(
		"Ambisonics encode → binaural decode\n"
		"Order " + ofToString(order) + " · " + ofToString(ofxSteamAudio::ambisonicsChannels(order)) + " channels\n"
		"Headphones recommended", 20, 30);
}

void ofApp::audioOut(ofSoundBuffer& buffer) {
	if (!ok) return;
	monoIn.fillSine(280.0f, (float)audioSettings.samplingRate, 0.45f, phase);
	IPLVector3 dir = ofxSteamAudio::relativeDirection(context, sourcePos);
	encode.apply(dir, order, monoIn.get(), ambi.get());
	ambiBin.apply(hrtf, order, ambi.get(), stereoOut.get());
	buffer.set(0);
	stereoOut.mixTo(buffer);
}
