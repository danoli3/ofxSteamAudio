#pragma once

#include "ofxSteamAudioUtils.h"

namespace ofxSteamAudio {

/// RAII wrapper for IPLHRTF (head-related transfer function database).
class HRTF {
public:
	HRTF() = default;
	~HRTF();

	HRTF(const HRTF&) = delete;
	HRTF& operator=(const HRTF&) = delete;
	HRTF(HRTF&& other) noexcept;
	HRTF& operator=(HRTF&& other) noexcept;

	bool create(IPLContext context, const IPLAudioSettings& audioSettings,
	            IPLHRTFType type = IPL_HRTFTYPE_DEFAULT,
	            float volume = 1.0f,
	            IPLHRTFNormType normType = IPL_HRTFNORMTYPE_NONE,
	            const char* sofaFileName = nullptr);

	void release();

	bool isValid() const { return hrtf != nullptr; }
	IPLHRTF get() const { return hrtf; }
	operator IPLHRTF() const { return hrtf; }

private:
	IPLHRTF hrtf = nullptr;
};

} // namespace ofxSteamAudio
