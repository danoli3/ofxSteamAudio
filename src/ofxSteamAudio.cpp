#include "ofxSteamAudio.h"

namespace ofxSteamAudio {

Engine::~Engine() {
	shutdown();
}

bool Engine::setup(int sr, int bs, bool trackMemory) {
	shutdown();
	sampleRate = sr;
	bufferSize = bs;
	audioSettings.samplingRate = sampleRate;
	audioSettings.frameSize = bufferSize;

	if (!context.setup(trackMemory)) return false;
	if (!hrtf.create(context, audioSettings)) return false;
	if (!binaural.create(context, audioSettings, hrtf)) return false;
	if (!tempIn.allocate(context, 1, bufferSize)) return false;
	if (!tempOut.allocate(context, 2, bufferSize)) return false;

	ready = true;
	ofLogNotice("ofxSteamAudio") << "Engine ready @ " << sampleRate << " Hz / " << bufferSize << " frames";
	return true;
}

void Engine::shutdown() {
	tempOut.free();
	tempIn.free();
	binaural.release();
	hrtf.release();
	context.release();
	sources.clear();
	ready = false;
}

void Engine::setListener(const glm::vec3& position, const glm::vec3& ahead, const glm::vec3& up) {
	listenerPos = position;
	listenerAhead = glm::normalize(ahead);
	listenerUp = glm::normalize(up);
}

void Engine::setListener(const ofCamera& cam) {
	setListener(cam.getPosition(), cam.getLookAtDir(), cam.getUpDir());
}

void Engine::setListener(const ofNode& node) {
	setListener(node.getGlobalPosition(), node.getLookAtDir(), node.getUpDir());
}

int Engine::addSource(const glm::vec3& position) {
	Source s;
	s.position = position;
	sources.push_back(s);
	return (int)sources.size() - 1;
}

void Engine::updateSource(int sourceId, const glm::vec3& position) {
	if (sourceId >= 0 && sourceId < (int)sources.size()) {
		sources[sourceId].position = position;
	}
}

void Engine::removeSource(int sourceId) {
	if (sourceId >= 0 && sourceId < (int)sources.size()) {
		sources[sourceId].active = false;
	}
}

void Engine::setSourceGain(int sourceId, float gain) {
	if (sourceId >= 0 && sourceId < (int)sources.size()) {
		sources[sourceId].gain = gain;
	}
}

void Engine::setSourceFrequency(int sourceId, float hz) {
	if (sourceId >= 0 && sourceId < (int)sources.size()) {
		sources[sourceId].frequency = hz;
	}
}

void Engine::processSource(int sourceId, IPLAudioBuffer* monoIn, ofSoundBuffer& outputBuffer, float mixGain) {
	if (!ready || sourceId < 0 || sourceId >= (int)sources.size()) return;
	const auto& src = sources[sourceId];
	if (!src.active) return;

	IPLVector3 dir = relativeDirection(context, src.position, listenerPos, listenerAhead, listenerUp);
	binaural.apply(dir, hrtf, monoIn, tempOut.get());
	tempOut.mixTo(outputBuffer, mixGain);
}

void Engine::processAudio(ofSoundBuffer& outputBuffer) {
	if (!ready) return;
	outputBuffer.set(0.0f);

	for (size_t i = 0; i < sources.size(); ++i) {
		auto& src = sources[i];
		if (!src.active) continue;

		tempIn.fillSine(src.frequency, (float)sampleRate, src.gain, src.phase);
		IPLVector3 dir = relativeDirection(context, src.position, listenerPos, listenerAhead, listenerUp);
		binaural.apply(dir, hrtf, tempIn.get(), tempOut.get());
		tempOut.mixTo(outputBuffer, 1.0f);
	}
}

} // namespace ofxSteamAudio
