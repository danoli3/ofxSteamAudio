#include "ofApp.h"

void ofApp::setup() {
	ofSetWindowTitle("ofxSteamAudio · Parametric Reflections");
	ofSetFrameRate(60);

	audioSettings.samplingRate = 44100;
	audioSettings.frameSize = 512;
	const int irSize = audioSettings.samplingRate; // 1s
	const int irCh = 1;

	ok = context.setup()
		&& hrtf.create(context, audioSettings)
		&& binaural.create(context, audioSettings, hrtf)
		&& reflection.create(context, audioSettings, IPL_REFLECTIONEFFECTTYPE_PARAMETRIC, irSize, irCh)
		&& monoIn.allocate(context, 1, audioSettings.frameSize)
		&& dryStereo.allocate(context, 2, audioSettings.frameSize)
		&& wetMono.allocate(context, 1, audioSettings.frameSize)
		&& wetStereo.allocate(context, 2, audioSettings.frameSize);
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
	sourcePos = glm::vec3(2.0f * cos(t * 0.5f), 0, 2.0f * sin(t * 0.5f));
}

void ofApp::draw() {
	ofBackground(16);
	ofDrawBitmapStringHighlight(
		"Parametric ReflectionEffect (artificial reverb)\n"
		"Keys: [ ] reverb time  - = wet/dry mix\n"
		"RT60~" + ofToString(reverbTime, 2) + "s  wet=" + ofToString(wetDry, 2), 20, 30);

	ofPushMatrix();
	ofTranslate(ofGetWidth() * 0.5f, ofGetHeight() * 0.55f);
	ofSetColor(90);
	ofDrawRectangle(-150, -100, 300, 200);
	ofSetColor(255, 140, 80);
	ofDrawCircle(sourcePos.x * 40, -sourcePos.z * 40, 12);
	ofSetColor(255);
	ofDrawBitmapString("room", -20, -110);
	ofPopMatrix();
}

void ofApp::audioOut(ofSoundBuffer& buffer) {
	if (!ok) return;
	monoIn.fillSine(250.0f, (float)audioSettings.samplingRate, 0.4f, phase);

	IPLVector3 dir = ofxSteamAudio::relativeDirection(context, sourcePos);
	binaural.apply(dir, hrtf, monoIn.get(), dryStereo.get());

	IPLReflectionEffectParams rp{};
	rp.type = IPL_REFLECTIONEFFECTTYPE_PARAMETRIC;
	rp.reverbTimes[0] = reverbTime;
	rp.reverbTimes[1] = reverbTime * 0.85f;
	rp.reverbTimes[2] = reverbTime * 0.7f;
	rp.numChannels = 1;
	rp.irSize = audioSettings.samplingRate;
	reflection.apply(rp, monoIn.get(), wetMono.get(), nullptr);

	// Upcook wet mono to stereo (simple duplicate) then mix
	buffer.set(0);
	dryStereo.mixTo(buffer, 1.0f - wetDry);
	for (int i = 0; i < audioSettings.frameSize; ++i) {
		float s = wetMono.channel(0)[i] * wetDry;
		buffer[i * 2 + 0] += s;
		buffer[i * 2 + 1] += s;
	}
}

void ofApp::keyPressed(int key) {
	if (key == '[') reverbTime = std::max(0.1f, reverbTime - 0.1f);
	if (key == ']') reverbTime = std::min(4.0f, reverbTime + 0.1f);
	if (key == '-') wetDry = std::max(0.0f, wetDry - 0.05f);
	if (key == '=' || key == '+') wetDry = std::min(1.0f, wetDry + 0.05f);
}
