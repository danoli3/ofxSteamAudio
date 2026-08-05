#include "ofxSteamAudioBuffer.h"
#include <cstring>
#include <cmath>

namespace ofxSteamAudio {

AudioBuffer::~AudioBuffer() {
	free();
}

AudioBuffer::AudioBuffer(AudioBuffer&& other) noexcept
	: ctx(other.ctx), buffer(other.buffer) {
	other.ctx = nullptr;
	other.buffer = {};
}

AudioBuffer& AudioBuffer::operator=(AudioBuffer&& other) noexcept {
	if (this != &other) {
		free();
		ctx = other.ctx;
		buffer = other.buffer;
		other.ctx = nullptr;
		other.buffer = {};
	}
	return *this;
}

bool AudioBuffer::allocate(IPLContext context, int numChannels, int numSamples) {
	free();
	ctx = context;
	return check(iplAudioBufferAllocate(context, numChannels, numSamples, &buffer),
	             "iplAudioBufferAllocate");
}

void AudioBuffer::free() {
	if (ctx && buffer.data) {
		iplAudioBufferFree(ctx, &buffer);
	}
	buffer = {};
	ctx = nullptr;
}

void AudioBuffer::clear() {
	if (!buffer.data) return;
	for (int c = 0; c < buffer.numChannels; ++c) {
		std::memset(buffer.data[c], 0, sizeof(IPLfloat32) * buffer.numSamples);
	}
}

void AudioBuffer::fillSine(float frequency, float sampleRate, float amplitude, float& phase) {
	if (!buffer.data || buffer.numChannels < 1) return;
	const float inc = frequency * TWO_PI / sampleRate;
	for (int i = 0; i < buffer.numSamples; ++i) {
		phase += inc;
		if (phase > TWO_PI) phase -= TWO_PI;
		const float s = std::sinf(phase) * amplitude;
		for (int c = 0; c < buffer.numChannels; ++c) {
			buffer.data[c][i] = s;
		}
	}
}

void AudioBuffer::copyTo(ofSoundBuffer& out, float scale) const {
	if (!buffer.data) return;
	const int frames = std::min(buffer.numSamples, (int)out.getNumFrames());
	const int outCh = (int)out.getNumChannels();
	for (int i = 0; i < frames; ++i) {
		for (int c = 0; c < outCh; ++c) {
			const int srcCh = std::min(c, buffer.numChannels - 1);
			out[i * outCh + c] = buffer.data[srcCh][i] * scale;
		}
	}
}

void AudioBuffer::mixTo(ofSoundBuffer& out, float scale) const {
	if (!buffer.data) return;
	const int frames = std::min(buffer.numSamples, (int)out.getNumFrames());
	const int outCh = (int)out.getNumChannels();
	for (int i = 0; i < frames; ++i) {
		for (int c = 0; c < outCh; ++c) {
			const int srcCh = std::min(c, buffer.numChannels - 1);
			out[i * outCh + c] += buffer.data[srcCh][i] * scale;
		}
	}
}

} // namespace ofxSteamAudio
