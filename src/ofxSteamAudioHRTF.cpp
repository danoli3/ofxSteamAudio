#include "ofxSteamAudioHRTF.h"

namespace ofxSteamAudio {

HRTF::~HRTF() { release(); }

HRTF::HRTF(HRTF&& other) noexcept : hrtf(other.hrtf) { other.hrtf = nullptr; }

HRTF& HRTF::operator=(HRTF&& other) noexcept {
	if (this != &other) {
		release();
		hrtf = other.hrtf;
		other.hrtf = nullptr;
	}
	return *this;
}

bool HRTF::create(IPLContext context, const IPLAudioSettings& audioSettings,
                  IPLHRTFType type, float volume, IPLHRTFNormType normType,
                  const char* sofaFileName) {
	release();
	IPLHRTFSettings settings{};
	settings.type = type;
	settings.volume = volume;
	settings.normType = normType;
	settings.sofaFileName = sofaFileName;
	IPLAudioSettings audio = audioSettings;
	return check(iplHRTFCreate(context, &audio, &settings, &hrtf), "iplHRTFCreate");
}

void HRTF::release() {
	if (hrtf) {
		iplHRTFRelease(&hrtf);
		hrtf = nullptr;
	}
}

} // namespace ofxSteamAudio
