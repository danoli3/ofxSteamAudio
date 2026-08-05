#pragma once

#include "ofxSteamAudioContext.h"

namespace ofxSteamAudio {

/// RAII wrapper for IPLAudioBuffer (deinterleaved float PCM).
class AudioBuffer {
public:
	AudioBuffer() = default;
	~AudioBuffer();

	AudioBuffer(const AudioBuffer&) = delete;
	AudioBuffer& operator=(const AudioBuffer&) = delete;
	AudioBuffer(AudioBuffer&& other) noexcept;
	AudioBuffer& operator=(AudioBuffer&& other) noexcept;

	bool allocate(IPLContext context, int numChannels, int numSamples);
	void free();

	bool isValid() const { return buffer.data != nullptr; }
	IPLAudioBuffer* get() { return &buffer; }
	const IPLAudioBuffer* get() const { return &buffer; }
	IPLAudioBuffer& ref() { return buffer; }

	int numChannels() const { return buffer.numChannels; }
	int numSamples() const { return buffer.numSamples; }
	IPLfloat32* channel(int c) { return buffer.data[c]; }

	void clear();
	void fillSine(float frequency, float sampleRate, float amplitude, float& phase);

	void interleave(IPLContext context, IPLfloat32* dst) {
		iplAudioBufferInterleave(context, &buffer, dst);
	}
	void deinterleave(IPLContext context, IPLfloat32* src) {
		iplAudioBufferDeinterleave(context, src, &buffer);
	}
	void mixInto(IPLContext context, AudioBuffer& mix) {
		iplAudioBufferMix(context, &buffer, mix.get());
	}
	void downmixFrom(IPLContext context, AudioBuffer& in) {
		iplAudioBufferDownmix(context, in.get(), &buffer);
	}
	void convertAmbisonics(IPLContext context, IPLAmbisonicsType inType, IPLAmbisonicsType outType, AudioBuffer& in) {
		iplAudioBufferConvertAmbisonics(context, inType, outType, in.get(), &buffer);
	}

	/// Copy deinterleaved data into ofSoundBuffer (interleaved stereo or mono).
	void copyTo(ofSoundBuffer& out, float scale = 1.0f) const;
	/// Mix deinterleaved data into ofSoundBuffer.
	void mixTo(ofSoundBuffer& out, float scale = 1.0f) const;

private:
	IPLContext ctx = nullptr;
	IPLAudioBuffer buffer{};
};

} // namespace ofxSteamAudio
