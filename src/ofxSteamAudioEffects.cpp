#include "ofxSteamAudioEffects.h"

namespace ofxSteamAudio {

// ---- move helpers ----
#define OFXSA_MOVE_IMPL(Class, Member) \
	Class::Class(Class&& o) noexcept : Member(o.Member) { o.Member = nullptr; } \
	Class& Class::operator=(Class&& o) noexcept { \
		if (this != &o) { release(); Member = o.Member; o.Member = nullptr; } \
		return *this; \
	}

// =============================================================================
// PanningEffect
// =============================================================================
PanningEffect::~PanningEffect() { release(); }
OFXSA_MOVE_IMPL(PanningEffect, effect)

bool PanningEffect::create(IPLContext context, const IPLAudioSettings& audioSettings, IPLSpeakerLayoutType layout) {
	release();
	IPLPanningEffectSettings s{};
	s.speakerLayout.type = layout;
	IPLAudioSettings audio = audioSettings;
	return check(iplPanningEffectCreate(context, &audio, &s, &effect), "iplPanningEffectCreate");
}
void PanningEffect::release() { if (effect) { iplPanningEffectRelease(&effect); effect = nullptr; } }
void PanningEffect::reset() { if (effect) iplPanningEffectReset(effect); }
IPLAudioEffectState PanningEffect::apply(const IPLVector3& direction, IPLAudioBuffer* in, IPLAudioBuffer* out) {
	IPLPanningEffectParams p{};
	p.direction = direction;
	return iplPanningEffectApply(effect, &p, in, out);
}
IPLAudioEffectState PanningEffect::getTail(IPLAudioBuffer* out) { return iplPanningEffectGetTail(effect, out); }
IPLint32 PanningEffect::getTailSize() const { return iplPanningEffectGetTailSize(effect); }

// =============================================================================
// BinauralEffect
// =============================================================================
BinauralEffect::~BinauralEffect() { release(); }
OFXSA_MOVE_IMPL(BinauralEffect, effect)

bool BinauralEffect::create(IPLContext context, const IPLAudioSettings& audioSettings, IPLHRTF hrtf) {
	release();
	IPLBinauralEffectSettings s{};
	s.hrtf = hrtf;
	IPLAudioSettings audio = audioSettings;
	return check(iplBinauralEffectCreate(context, &audio, &s, &effect), "iplBinauralEffectCreate");
}
void BinauralEffect::release() { if (effect) { iplBinauralEffectRelease(&effect); effect = nullptr; } }
void BinauralEffect::reset() { if (effect) iplBinauralEffectReset(effect); }
IPLAudioEffectState BinauralEffect::apply(const IPLBinauralEffectParams& params, IPLAudioBuffer* in, IPLAudioBuffer* out) {
	IPLBinauralEffectParams p = params;
	return iplBinauralEffectApply(effect, &p, in, out);
}
IPLAudioEffectState BinauralEffect::apply(const IPLVector3& direction, IPLHRTF hrtf,
                                          IPLAudioBuffer* in, IPLAudioBuffer* out,
                                          IPLHRTFInterpolation interpolation, float spatialBlend) {
	IPLBinauralEffectParams p{};
	p.direction = direction;
	p.interpolation = interpolation;
	p.spatialBlend = spatialBlend;
	p.hrtf = hrtf;
	return iplBinauralEffectApply(effect, &p, in, out);
}
IPLAudioEffectState BinauralEffect::getTail(IPLAudioBuffer* out) { return iplBinauralEffectGetTail(effect, out); }
IPLint32 BinauralEffect::getTailSize() const { return iplBinauralEffectGetTailSize(effect); }

// =============================================================================
// VirtualSurroundEffect
// =============================================================================
VirtualSurroundEffect::~VirtualSurroundEffect() { release(); }
OFXSA_MOVE_IMPL(VirtualSurroundEffect, effect)

bool VirtualSurroundEffect::create(IPLContext context, const IPLAudioSettings& audioSettings,
                                   IPLHRTF hrtf, IPLSpeakerLayoutType layout) {
	release();
	IPLVirtualSurroundEffectSettings s{};
	s.speakerLayout.type = layout;
	s.hrtf = hrtf;
	IPLAudioSettings audio = audioSettings;
	return check(iplVirtualSurroundEffectCreate(context, &audio, &s, &effect), "iplVirtualSurroundEffectCreate");
}
void VirtualSurroundEffect::release() { if (effect) { iplVirtualSurroundEffectRelease(&effect); effect = nullptr; } }
void VirtualSurroundEffect::reset() { if (effect) iplVirtualSurroundEffectReset(effect); }
IPLAudioEffectState VirtualSurroundEffect::apply(IPLHRTF hrtf, IPLAudioBuffer* in, IPLAudioBuffer* out) {
	IPLVirtualSurroundEffectParams p{};
	p.hrtf = hrtf;
	return iplVirtualSurroundEffectApply(effect, &p, in, out);
}
IPLAudioEffectState VirtualSurroundEffect::getTail(IPLAudioBuffer* out) { return iplVirtualSurroundEffectGetTail(effect, out); }
IPLint32 VirtualSurroundEffect::getTailSize() const { return iplVirtualSurroundEffectGetTailSize(effect); }

// =============================================================================
// AmbisonicsEncodeEffect
// =============================================================================
AmbisonicsEncodeEffect::~AmbisonicsEncodeEffect() { release(); }
OFXSA_MOVE_IMPL(AmbisonicsEncodeEffect, effect)

bool AmbisonicsEncodeEffect::create(IPLContext context, const IPLAudioSettings& audioSettings, int maxOrder) {
	release();
	IPLAmbisonicsEncodeEffectSettings s{};
	s.maxOrder = maxOrder;
	IPLAudioSettings audio = audioSettings;
	return check(iplAmbisonicsEncodeEffectCreate(context, &audio, &s, &effect), "iplAmbisonicsEncodeEffectCreate");
}
void AmbisonicsEncodeEffect::release() { if (effect) { iplAmbisonicsEncodeEffectRelease(&effect); effect = nullptr; } }
void AmbisonicsEncodeEffect::reset() { if (effect) iplAmbisonicsEncodeEffectReset(effect); }
IPLAudioEffectState AmbisonicsEncodeEffect::apply(const IPLVector3& direction, int order,
                                                  IPLAudioBuffer* in, IPLAudioBuffer* out) {
	IPLAmbisonicsEncodeEffectParams p{};
	p.direction = direction;
	p.order = order;
	return iplAmbisonicsEncodeEffectApply(effect, &p, in, out);
}
IPLAudioEffectState AmbisonicsEncodeEffect::getTail(IPLAudioBuffer* out) { return iplAmbisonicsEncodeEffectGetTail(effect, out); }
IPLint32 AmbisonicsEncodeEffect::getTailSize() const { return iplAmbisonicsEncodeEffectGetTailSize(effect); }

// =============================================================================
// AmbisonicsPanningEffect
// =============================================================================
AmbisonicsPanningEffect::~AmbisonicsPanningEffect() { release(); }
OFXSA_MOVE_IMPL(AmbisonicsPanningEffect, effect)

bool AmbisonicsPanningEffect::create(IPLContext context, const IPLAudioSettings& audioSettings,
                                     IPLSpeakerLayoutType layout, int maxOrder) {
	release();
	IPLAmbisonicsPanningEffectSettings s{};
	s.speakerLayout.type = layout;
	s.maxOrder = maxOrder;
	IPLAudioSettings audio = audioSettings;
	return check(iplAmbisonicsPanningEffectCreate(context, &audio, &s, &effect), "iplAmbisonicsPanningEffectCreate");
}
void AmbisonicsPanningEffect::release() { if (effect) { iplAmbisonicsPanningEffectRelease(&effect); effect = nullptr; } }
void AmbisonicsPanningEffect::reset() { if (effect) iplAmbisonicsPanningEffectReset(effect); }
IPLAudioEffectState AmbisonicsPanningEffect::apply(int order, IPLAudioBuffer* in, IPLAudioBuffer* out) {
	IPLAmbisonicsPanningEffectParams p{};
	p.order = order;
	return iplAmbisonicsPanningEffectApply(effect, &p, in, out);
}
IPLAudioEffectState AmbisonicsPanningEffect::getTail(IPLAudioBuffer* out) { return iplAmbisonicsPanningEffectGetTail(effect, out); }
IPLint32 AmbisonicsPanningEffect::getTailSize() const { return iplAmbisonicsPanningEffectGetTailSize(effect); }

// =============================================================================
// AmbisonicsBinauralEffect
// =============================================================================
AmbisonicsBinauralEffect::~AmbisonicsBinauralEffect() { release(); }
OFXSA_MOVE_IMPL(AmbisonicsBinauralEffect, effect)

bool AmbisonicsBinauralEffect::create(IPLContext context, const IPLAudioSettings& audioSettings, IPLHRTF hrtf, int maxOrder) {
	release();
	IPLAmbisonicsBinauralEffectSettings s{};
	s.hrtf = hrtf;
	s.maxOrder = maxOrder;
	IPLAudioSettings audio = audioSettings;
	return check(iplAmbisonicsBinauralEffectCreate(context, &audio, &s, &effect), "iplAmbisonicsBinauralEffectCreate");
}
void AmbisonicsBinauralEffect::release() { if (effect) { iplAmbisonicsBinauralEffectRelease(&effect); effect = nullptr; } }
void AmbisonicsBinauralEffect::reset() { if (effect) iplAmbisonicsBinauralEffectReset(effect); }
IPLAudioEffectState AmbisonicsBinauralEffect::apply(IPLHRTF hrtf, int order, IPLAudioBuffer* in, IPLAudioBuffer* out) {
	IPLAmbisonicsBinauralEffectParams p{};
	p.hrtf = hrtf;
	p.order = order;
	return iplAmbisonicsBinauralEffectApply(effect, &p, in, out);
}
IPLAudioEffectState AmbisonicsBinauralEffect::getTail(IPLAudioBuffer* out) { return iplAmbisonicsBinauralEffectGetTail(effect, out); }
IPLint32 AmbisonicsBinauralEffect::getTailSize() const { return iplAmbisonicsBinauralEffectGetTailSize(effect); }

// =============================================================================
// AmbisonicsRotationEffect
// =============================================================================
AmbisonicsRotationEffect::~AmbisonicsRotationEffect() { release(); }
OFXSA_MOVE_IMPL(AmbisonicsRotationEffect, effect)

bool AmbisonicsRotationEffect::create(IPLContext context, const IPLAudioSettings& audioSettings, int maxOrder) {
	release();
	IPLAmbisonicsRotationEffectSettings s{};
	s.maxOrder = maxOrder;
	IPLAudioSettings audio = audioSettings;
	return check(iplAmbisonicsRotationEffectCreate(context, &audio, &s, &effect), "iplAmbisonicsRotationEffectCreate");
}
void AmbisonicsRotationEffect::release() { if (effect) { iplAmbisonicsRotationEffectRelease(&effect); effect = nullptr; } }
void AmbisonicsRotationEffect::reset() { if (effect) iplAmbisonicsRotationEffectReset(effect); }
IPLAudioEffectState AmbisonicsRotationEffect::apply(const IPLCoordinateSpace3& orientation, int order,
                                                    IPLAudioBuffer* in, IPLAudioBuffer* out) {
	IPLAmbisonicsRotationEffectParams p{};
	p.orientation = orientation;
	p.order = order;
	return iplAmbisonicsRotationEffectApply(effect, &p, in, out);
}
IPLAudioEffectState AmbisonicsRotationEffect::getTail(IPLAudioBuffer* out) { return iplAmbisonicsRotationEffectGetTail(effect, out); }
IPLint32 AmbisonicsRotationEffect::getTailSize() const { return iplAmbisonicsRotationEffectGetTailSize(effect); }

// =============================================================================
// AmbisonicsDecodeEffect
// =============================================================================
AmbisonicsDecodeEffect::~AmbisonicsDecodeEffect() { release(); }
OFXSA_MOVE_IMPL(AmbisonicsDecodeEffect, effect)

bool AmbisonicsDecodeEffect::create(IPLContext context, const IPLAudioSettings& audioSettings,
                                    IPLSpeakerLayoutType layout, IPLHRTF hrtf, int maxOrder) {
	release();
	IPLAmbisonicsDecodeEffectSettings s{};
	s.speakerLayout.type = layout;
	s.hrtf = hrtf;
	s.maxOrder = maxOrder;
	IPLAudioSettings audio = audioSettings;
	return check(iplAmbisonicsDecodeEffectCreate(context, &audio, &s, &effect), "iplAmbisonicsDecodeEffectCreate");
}
void AmbisonicsDecodeEffect::release() { if (effect) { iplAmbisonicsDecodeEffectRelease(&effect); effect = nullptr; } }
void AmbisonicsDecodeEffect::reset() { if (effect) iplAmbisonicsDecodeEffectReset(effect); }
IPLAudioEffectState AmbisonicsDecodeEffect::apply(const IPLAmbisonicsDecodeEffectParams& params,
                                                  IPLAudioBuffer* in, IPLAudioBuffer* out) {
	IPLAmbisonicsDecodeEffectParams p = params;
	return iplAmbisonicsDecodeEffectApply(effect, &p, in, out);
}
IPLAudioEffectState AmbisonicsDecodeEffect::getTail(IPLAudioBuffer* out) { return iplAmbisonicsDecodeEffectGetTail(effect, out); }
IPLint32 AmbisonicsDecodeEffect::getTailSize() const { return iplAmbisonicsDecodeEffectGetTailSize(effect); }

// =============================================================================
// DirectEffect
// =============================================================================
DirectEffect::~DirectEffect() { release(); }
OFXSA_MOVE_IMPL(DirectEffect, effect)

bool DirectEffect::create(IPLContext context, const IPLAudioSettings& audioSettings, int numChannels) {
	release();
	IPLDirectEffectSettings s{};
	s.numChannels = numChannels;
	IPLAudioSettings audio = audioSettings;
	return check(iplDirectEffectCreate(context, &audio, &s, &effect), "iplDirectEffectCreate");
}
void DirectEffect::release() { if (effect) { iplDirectEffectRelease(&effect); effect = nullptr; } }
void DirectEffect::reset() { if (effect) iplDirectEffectReset(effect); }
IPLAudioEffectState DirectEffect::apply(const IPLDirectEffectParams& params, IPLAudioBuffer* in, IPLAudioBuffer* out) {
	IPLDirectEffectParams p = params;
	return iplDirectEffectApply(effect, &p, in, out);
}
IPLAudioEffectState DirectEffect::getTail(IPLAudioBuffer* out) { return iplDirectEffectGetTail(effect, out); }
IPLint32 DirectEffect::getTailSize() const { return iplDirectEffectGetTailSize(effect); }

// =============================================================================
// ReflectionEffect
// =============================================================================
ReflectionEffect::~ReflectionEffect() { release(); }
OFXSA_MOVE_IMPL(ReflectionEffect, effect)

bool ReflectionEffect::create(IPLContext context, const IPLAudioSettings& audioSettings,
                              IPLReflectionEffectType type, int irSize, int numChannels) {
	release();
	IPLReflectionEffectSettings s{};
	s.type = type;
	s.irSize = irSize;
	s.numChannels = numChannels;
	IPLAudioSettings audio = audioSettings;
	return check(iplReflectionEffectCreate(context, &audio, &s, &effect), "iplReflectionEffectCreate");
}
void ReflectionEffect::release() { if (effect) { iplReflectionEffectRelease(&effect); effect = nullptr; } }
void ReflectionEffect::reset() { if (effect) iplReflectionEffectReset(effect); }
IPLAudioEffectState ReflectionEffect::apply(const IPLReflectionEffectParams& params,
                                            IPLAudioBuffer* in, IPLAudioBuffer* out,
                                            IPLReflectionMixer mixer) {
	IPLReflectionEffectParams p = params;
	return iplReflectionEffectApply(effect, &p, in, out, mixer);
}
IPLAudioEffectState ReflectionEffect::getTail(IPLAudioBuffer* out, IPLReflectionMixer mixer) {
	return iplReflectionEffectGetTail(effect, out, mixer);
}
IPLint32 ReflectionEffect::getTailSize() const { return iplReflectionEffectGetTailSize(effect); }

// =============================================================================
// ReflectionMixer
// =============================================================================
ReflectionMixer::~ReflectionMixer() { release(); }
OFXSA_MOVE_IMPL(ReflectionMixer, mixer)

bool ReflectionMixer::create(IPLContext context, const IPLAudioSettings& audioSettings,
                             IPLReflectionEffectType type, int irSize, int numChannels) {
	release();
	IPLReflectionEffectSettings s{};
	s.type = type;
	s.irSize = irSize;
	s.numChannels = numChannels;
	IPLAudioSettings audio = audioSettings;
	return check(iplReflectionMixerCreate(context, &audio, &s, &mixer), "iplReflectionMixerCreate");
}
void ReflectionMixer::release() { if (mixer) { iplReflectionMixerRelease(&mixer); mixer = nullptr; } }
void ReflectionMixer::reset() { if (mixer) iplReflectionMixerReset(mixer); }
IPLAudioEffectState ReflectionMixer::apply(const IPLReflectionEffectParams& params, IPLAudioBuffer* out) {
	IPLReflectionEffectParams p = params;
	return iplReflectionMixerApply(mixer, &p, out);
}

// =============================================================================
// PathEffect
// =============================================================================
PathEffect::~PathEffect() { release(); }
OFXSA_MOVE_IMPL(PathEffect, effect)

bool PathEffect::create(IPLContext context, const IPLAudioSettings& audioSettings,
                        int maxOrder, bool spatialize, IPLHRTF hrtf, IPLSpeakerLayoutType layout) {
	release();
	IPLPathEffectSettings s{};
	s.maxOrder = maxOrder;
	s.spatialize = spatialize ? IPL_TRUE : IPL_FALSE;
	s.speakerLayout.type = layout;
	s.hrtf = hrtf;
	IPLAudioSettings audio = audioSettings;
	return check(iplPathEffectCreate(context, &audio, &s, &effect), "iplPathEffectCreate");
}
void PathEffect::release() { if (effect) { iplPathEffectRelease(&effect); effect = nullptr; } }
void PathEffect::reset() { if (effect) iplPathEffectReset(effect); }
IPLAudioEffectState PathEffect::apply(const IPLPathEffectParams& params, IPLAudioBuffer* in, IPLAudioBuffer* out) {
	IPLPathEffectParams p = params;
	return iplPathEffectApply(effect, &p, in, out);
}
IPLAudioEffectState PathEffect::getTail(IPLAudioBuffer* out) { return iplPathEffectGetTail(effect, out); }
IPLint32 PathEffect::getTailSize() const { return iplPathEffectGetTailSize(effect); }

#undef OFXSA_MOVE_IMPL

} // namespace ofxSteamAudio
