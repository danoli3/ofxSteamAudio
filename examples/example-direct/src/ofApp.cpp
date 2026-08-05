#include "ofApp.h"

void ofApp::setup() {
	ofSetWindowTitle("ofxSteamAudio · Direct + Occlusion");
	ofSetFrameRate(60);

	audioSettings.samplingRate = 44100;
	audioSettings.frameSize = 512;

	ok = context.setup()
		&& hrtf.create(context, audioSettings)
		&& binaural.create(context, audioSettings, hrtf)
		&& direct.create(context, audioSettings, 1)
		&& monoIn.allocate(context, 1, audioSettings.frameSize)
		&& directOut.allocate(context, 1, audioSettings.frameSize)
		&& stereoOut.allocate(context, 2, audioSettings.frameSize)
		&& scene.create(context)
		&& simulator.create(context, audioSettings, IPL_SIMULATIONFLAGS_DIRECT);

	if (!ok) return;

	// Wall between listener (z=0) and far source (z=-8)
	scene.addBox(6, 3, 0.4f, ofxSteamAudio::Materials::concrete(), glm::vec3(0, 1.5f, -3));
	scene.addGroundPlane(30.f);
	scene.commit();

	simulator.setScene(scene);
	simulator.commit();
	simSource.create(simulator.get(), IPL_SIMULATIONFLAGS_DIRECT);
	simSource.add(simulator.get());
	simulator.commit();

	ofSoundStreamSettings s;
	s.numOutputChannels = 2;
	s.sampleRate = audioSettings.samplingRate;
	s.bufferSize = audioSettings.frameSize;
	s.setOutListener(this);
	soundStream.setup(s);
}

void ofApp::update() {
	if (!ok) return;
	float t = ofGetElapsedTimef();
	// Move source left-right so it occasionally peeks around the wall
	sourcePos = glm::vec3(4.0f * sin(t * 0.5f), 1.5f, -8.0f);

	IPLSimulationSharedInputs shared{};
	shared.listener = ofxSteamAudio::makeCoordinateSpace(listenerPos);
	shared.numRays = 0;
	shared.numBounces = 0;
	shared.duration = 1.0f;
	shared.order = 0;
	shared.irradianceMinDistance = 1.0f;
	simulator.setSharedInputs(IPL_SIMULATIONFLAGS_DIRECT, &shared);

	IPLSimulationInputs inputs{};
	inputs.flags = IPL_SIMULATIONFLAGS_DIRECT;
	inputs.directFlags = static_cast<IPLDirectSimulationFlags>(
		IPL_DIRECTSIMULATIONFLAGS_DISTANCEATTENUATION |
		IPL_DIRECTSIMULATIONFLAGS_AIRABSORPTION |
		IPL_DIRECTSIMULATIONFLAGS_OCCLUSION |
		IPL_DIRECTSIMULATIONFLAGS_TRANSMISSION);
	inputs.source = ofxSteamAudio::makeCoordinateSpace(sourcePos);
	inputs.distanceAttenuationModel.type = IPL_DISTANCEATTENUATIONTYPE_DEFAULT;
	inputs.airAbsorptionModel.type = IPL_AIRABSORPTIONTYPE_DEFAULT;
	inputs.occlusionType = IPL_OCCLUSIONTYPE_RAYCAST;
	inputs.occlusionRadius = 0.1f;
	inputs.numOcclusionSamples = 4;
	simSource.setInputs(IPL_SIMULATIONFLAGS_DIRECT, &inputs);

	simulator.runDirect();
	IPLSimulationOutputs outputs{};
	simSource.getOutputs(IPL_SIMULATIONFLAGS_DIRECT, &outputs);
	lastOcclusion = outputs.direct.occlusion;
	lastAttenuation = outputs.direct.distanceAttenuation;
}

void ofApp::draw() {
	ofBackground(22);
	ofPushMatrix();
	ofTranslate(ofGetWidth() * 0.5f, ofGetHeight() * 0.55f);
	// top-down: x horizontal, -z up on screen
	auto toScreen = [](glm::vec3 p) { return glm::vec2(p.x * 30, p.z * 30); };
	ofSetColor(100);
	ofDrawRectangle(toScreen(glm::vec3(-3, 0, -3.2f)).x, toScreen(glm::vec3(0, 0, -3)).y - 6, 180, 12);
	ofSetColor(80, 180, 255);
	ofDrawCircle(toScreen(listenerPos), 10);
	ofSetColor(255, 120, 80);
	ofDrawCircle(toScreen(sourcePos), 10);
	ofPopMatrix();

	ofDrawBitmapStringHighlight(
		"DirectEffect + raycast occlusion\n"
		"Wall blocks path when source is behind it\n"
		"attenuation=" + ofToString(lastAttenuation, 2) +
		"  occlusion=" + ofToString(lastOcclusion, 2), 20, 30);
}

void ofApp::audioOut(ofSoundBuffer& buffer) {
	if (!ok) return;
	monoIn.fillSine(300.0f, (float)audioSettings.samplingRate, 0.55f, phase);

	IPLSimulationOutputs outputs{};
	simSource.getOutputs(IPL_SIMULATIONFLAGS_DIRECT, &outputs);

	IPLDirectEffectParams dp = outputs.direct;
	dp.flags = static_cast<IPLDirectEffectFlags>(
		IPL_DIRECTEFFECTFLAGS_APPLYDISTANCEATTENUATION |
		IPL_DIRECTEFFECTFLAGS_APPLYAIRABSORPTION |
		IPL_DIRECTEFFECTFLAGS_APPLYOCCLUSION |
		IPL_DIRECTEFFECTFLAGS_APPLYTRANSMISSION);
	dp.transmissionType = IPL_TRANSMISSIONTYPE_FREQINDEPENDENT;
	direct.apply(dp, monoIn.get(), directOut.get());

	IPLVector3 dir = ofxSteamAudio::relativeDirection(context, sourcePos, listenerPos);
	binaural.apply(dir, hrtf, directOut.get(), stereoOut.get());
	buffer.set(0);
	stereoOut.mixTo(buffer);
}
