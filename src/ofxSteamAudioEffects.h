#pragma once

#include "ofxSteamAudioUtils.h"
#include "ofxSteamAudioBuffer.h"

namespace ofxSteamAudio {

// -----------------------------------------------------------------------------
// Panning (mono → speaker layout)
// Maps to Steam Audio itest: panningeffect
// -----------------------------------------------------------------------------
class PanningEffect {
public:
	PanningEffect() = default;
	~PanningEffect();
	PanningEffect(const PanningEffect&) = delete;
	PanningEffect& operator=(const PanningEffect&) = delete;
	PanningEffect(PanningEffect&& o) noexcept;
	PanningEffect& operator=(PanningEffect&& o) noexcept;

	bool create(IPLContext context, const IPLAudioSettings& audioSettings,
	            IPLSpeakerLayoutType layout = IPL_SPEAKERLAYOUTTYPE_STEREO);
	void release();
	void reset();

	IPLAudioEffectState apply(const IPLVector3& direction, IPLAudioBuffer* in, IPLAudioBuffer* out);
	IPLAudioEffectState getTail(IPLAudioBuffer* out);
	IPLint32 getTailSize() const;

	bool isValid() const { return effect != nullptr; }
	IPLPanningEffect get() const { return effect; }

private:
	IPLPanningEffect effect = nullptr;
};

// -----------------------------------------------------------------------------
// Binaural (HRTF spatialization)
// Maps to Steam Audio itest: binauraleffect
// -----------------------------------------------------------------------------
class BinauralEffect {
public:
	BinauralEffect() = default;
	~BinauralEffect();
	BinauralEffect(const BinauralEffect&) = delete;
	BinauralEffect& operator=(const BinauralEffect&) = delete;
	BinauralEffect(BinauralEffect&& o) noexcept;
	BinauralEffect& operator=(BinauralEffect&& o) noexcept;

	bool create(IPLContext context, const IPLAudioSettings& audioSettings, IPLHRTF hrtf);
	void release();
	void reset();

	IPLAudioEffectState apply(const IPLBinauralEffectParams& params, IPLAudioBuffer* in, IPLAudioBuffer* out);
	IPLAudioEffectState apply(const IPLVector3& direction, IPLHRTF hrtf,
	                          IPLAudioBuffer* in, IPLAudioBuffer* out,
	                          IPLHRTFInterpolation interpolation = IPL_HRTFINTERPOLATION_NEAREST,
	                          float spatialBlend = 1.0f);
	IPLAudioEffectState getTail(IPLAudioBuffer* out);
	IPLint32 getTailSize() const;

	bool isValid() const { return effect != nullptr; }
	IPLBinauralEffect get() const { return effect; }

private:
	IPLBinauralEffect effect = nullptr;
};

// -----------------------------------------------------------------------------
// Virtual Surround (multi-channel → binaural)
// Maps to Steam Audio itest: virtualsurroundeffect
// -----------------------------------------------------------------------------
class VirtualSurroundEffect {
public:
	VirtualSurroundEffect() = default;
	~VirtualSurroundEffect();
	VirtualSurroundEffect(const VirtualSurroundEffect&) = delete;
	VirtualSurroundEffect& operator=(const VirtualSurroundEffect&) = delete;
	VirtualSurroundEffect(VirtualSurroundEffect&& o) noexcept;
	VirtualSurroundEffect& operator=(VirtualSurroundEffect&& o) noexcept;

	bool create(IPLContext context, const IPLAudioSettings& audioSettings,
	            IPLHRTF hrtf, IPLSpeakerLayoutType layout = IPL_SPEAKERLAYOUTTYPE_SURROUND_5_1);
	void release();
	void reset();

	IPLAudioEffectState apply(IPLHRTF hrtf, IPLAudioBuffer* in, IPLAudioBuffer* out);
	IPLAudioEffectState getTail(IPLAudioBuffer* out);
	IPLint32 getTailSize() const;

	bool isValid() const { return effect != nullptr; }
	IPLVirtualSurroundEffect get() const { return effect; }

private:
	IPLVirtualSurroundEffect effect = nullptr;
};

// -----------------------------------------------------------------------------
// Ambisonics Encode (mono point source → Ambisonics)
// -----------------------------------------------------------------------------
class AmbisonicsEncodeEffect {
public:
	AmbisonicsEncodeEffect() = default;
	~AmbisonicsEncodeEffect();
	AmbisonicsEncodeEffect(const AmbisonicsEncodeEffect&) = delete;
	AmbisonicsEncodeEffect& operator=(const AmbisonicsEncodeEffect&) = delete;
	AmbisonicsEncodeEffect(AmbisonicsEncodeEffect&& o) noexcept;
	AmbisonicsEncodeEffect& operator=(AmbisonicsEncodeEffect&& o) noexcept;

	bool create(IPLContext context, const IPLAudioSettings& audioSettings, int maxOrder);
	void release();
	void reset();

	IPLAudioEffectState apply(const IPLVector3& direction, int order,
	                          IPLAudioBuffer* in, IPLAudioBuffer* out);
	IPLAudioEffectState getTail(IPLAudioBuffer* out);
	IPLint32 getTailSize() const;

	bool isValid() const { return effect != nullptr; }
	IPLAmbisonicsEncodeEffect get() const { return effect; }

private:
	IPLAmbisonicsEncodeEffect effect = nullptr;
};

// -----------------------------------------------------------------------------
// Ambisonics Panning (Ambisonics → speakers)
// Maps to Steam Audio itest: ambisonicspanningeffect
// -----------------------------------------------------------------------------
class AmbisonicsPanningEffect {
public:
	AmbisonicsPanningEffect() = default;
	~AmbisonicsPanningEffect();
	AmbisonicsPanningEffect(const AmbisonicsPanningEffect&) = delete;
	AmbisonicsPanningEffect& operator=(const AmbisonicsPanningEffect&) = delete;
	AmbisonicsPanningEffect(AmbisonicsPanningEffect&& o) noexcept;
	AmbisonicsPanningEffect& operator=(AmbisonicsPanningEffect&& o) noexcept;

	bool create(IPLContext context, const IPLAudioSettings& audioSettings,
	            IPLSpeakerLayoutType layout, int maxOrder);
	void release();
	void reset();

	IPLAudioEffectState apply(int order, IPLAudioBuffer* in, IPLAudioBuffer* out);
	IPLAudioEffectState getTail(IPLAudioBuffer* out);
	IPLint32 getTailSize() const;

	bool isValid() const { return effect != nullptr; }
	IPLAmbisonicsPanningEffect get() const { return effect; }

private:
	IPLAmbisonicsPanningEffect effect = nullptr;
};

// -----------------------------------------------------------------------------
// Ambisonics Binaural (Ambisonics → HRTF binaural)
// Maps to Steam Audio itest: ambisonicsbinauraleffect
// -----------------------------------------------------------------------------
class AmbisonicsBinauralEffect {
public:
	AmbisonicsBinauralEffect() = default;
	~AmbisonicsBinauralEffect();
	AmbisonicsBinauralEffect(const AmbisonicsBinauralEffect&) = delete;
	AmbisonicsBinauralEffect& operator=(const AmbisonicsBinauralEffect&) = delete;
	AmbisonicsBinauralEffect(AmbisonicsBinauralEffect&& o) noexcept;
	AmbisonicsBinauralEffect& operator=(AmbisonicsBinauralEffect&& o) noexcept;

	bool create(IPLContext context, const IPLAudioSettings& audioSettings, IPLHRTF hrtf, int maxOrder);
	void release();
	void reset();

	IPLAudioEffectState apply(IPLHRTF hrtf, int order, IPLAudioBuffer* in, IPLAudioBuffer* out);
	IPLAudioEffectState getTail(IPLAudioBuffer* out);
	IPLint32 getTailSize() const;

	bool isValid() const { return effect != nullptr; }
	IPLAmbisonicsBinauralEffect get() const { return effect; }

private:
	IPLAmbisonicsBinauralEffect effect = nullptr;
};

// -----------------------------------------------------------------------------
// Ambisonics Rotation
// Maps to Steam Audio itest: ambisonicsrotateeffect
// -----------------------------------------------------------------------------
class AmbisonicsRotationEffect {
public:
	AmbisonicsRotationEffect() = default;
	~AmbisonicsRotationEffect();
	AmbisonicsRotationEffect(const AmbisonicsRotationEffect&) = delete;
	AmbisonicsRotationEffect& operator=(const AmbisonicsRotationEffect&) = delete;
	AmbisonicsRotationEffect(AmbisonicsRotationEffect&& o) noexcept;
	AmbisonicsRotationEffect& operator=(AmbisonicsRotationEffect&& o) noexcept;

	bool create(IPLContext context, const IPLAudioSettings& audioSettings, int maxOrder);
	void release();
	void reset();

	IPLAudioEffectState apply(const IPLCoordinateSpace3& orientation, int order,
	                          IPLAudioBuffer* in, IPLAudioBuffer* out);
	IPLAudioEffectState getTail(IPLAudioBuffer* out);
	IPLint32 getTailSize() const;

	bool isValid() const { return effect != nullptr; }
	IPLAmbisonicsRotationEffect get() const { return effect; }

private:
	IPLAmbisonicsRotationEffect effect = nullptr;
};

// -----------------------------------------------------------------------------
// Ambisonics Decode (Ambisonics → speakers or binaural via decode)
// -----------------------------------------------------------------------------
class AmbisonicsDecodeEffect {
public:
	AmbisonicsDecodeEffect() = default;
	~AmbisonicsDecodeEffect();
	AmbisonicsDecodeEffect(const AmbisonicsDecodeEffect&) = delete;
	AmbisonicsDecodeEffect& operator=(const AmbisonicsDecodeEffect&) = delete;
	AmbisonicsDecodeEffect(AmbisonicsDecodeEffect&& o) noexcept;
	AmbisonicsDecodeEffect& operator=(AmbisonicsDecodeEffect&& o) noexcept;

	bool create(IPLContext context, const IPLAudioSettings& audioSettings,
	            IPLSpeakerLayoutType layout, IPLHRTF hrtf, int maxOrder);
	void release();
	void reset();

	IPLAudioEffectState apply(const IPLAmbisonicsDecodeEffectParams& params,
	                          IPLAudioBuffer* in, IPLAudioBuffer* out);
	IPLAudioEffectState getTail(IPLAudioBuffer* out);
	IPLint32 getTailSize() const;

	bool isValid() const { return effect != nullptr; }
	IPLAmbisonicsDecodeEffect get() const { return effect; }

private:
	IPLAmbisonicsDecodeEffect effect = nullptr;
};

// -----------------------------------------------------------------------------
// Direct Effect (distance, air absorption, directivity, occlusion, transmission)
// Maps to Steam Audio itest: directsoundeffect / directsimulator
// -----------------------------------------------------------------------------
class DirectEffect {
public:
	DirectEffect() = default;
	~DirectEffect();
	DirectEffect(const DirectEffect&) = delete;
	DirectEffect& operator=(const DirectEffect&) = delete;
	DirectEffect(DirectEffect&& o) noexcept;
	DirectEffect& operator=(DirectEffect&& o) noexcept;

	bool create(IPLContext context, const IPLAudioSettings& audioSettings, int numChannels = 1);
	void release();
	void reset();

	IPLAudioEffectState apply(const IPLDirectEffectParams& params, IPLAudioBuffer* in, IPLAudioBuffer* out);
	IPLAudioEffectState getTail(IPLAudioBuffer* out);
	IPLint32 getTailSize() const;

	bool isValid() const { return effect != nullptr; }
	IPLDirectEffect get() const { return effect; }

private:
	IPLDirectEffect effect = nullptr;
};

// -----------------------------------------------------------------------------
// Reflection Effect (convolution / parametric / hybrid reverb)
// Maps to Steam Audio itest: reverbeffect, parametricreverb, hybridreverbeffect, convolutioneffect
// -----------------------------------------------------------------------------
class ReflectionEffect {
public:
	ReflectionEffect() = default;
	~ReflectionEffect();
	ReflectionEffect(const ReflectionEffect&) = delete;
	ReflectionEffect& operator=(const ReflectionEffect&) = delete;
	ReflectionEffect(ReflectionEffect&& o) noexcept;
	ReflectionEffect& operator=(ReflectionEffect&& o) noexcept;

	bool create(IPLContext context, const IPLAudioSettings& audioSettings,
	            IPLReflectionEffectType type, int irSize, int numChannels);
	void release();
	void reset();

	IPLAudioEffectState apply(const IPLReflectionEffectParams& params,
	                          IPLAudioBuffer* in, IPLAudioBuffer* out,
	                          IPLReflectionMixer mixer = nullptr);
	IPLAudioEffectState getTail(IPLAudioBuffer* out, IPLReflectionMixer mixer = nullptr);
	IPLint32 getTailSize() const;

	bool isValid() const { return effect != nullptr; }
	IPLReflectionEffect get() const { return effect; }

private:
	IPLReflectionEffect effect = nullptr;
};

// -----------------------------------------------------------------------------
// Reflection Mixer
// -----------------------------------------------------------------------------
class ReflectionMixer {
public:
	ReflectionMixer() = default;
	~ReflectionMixer();
	ReflectionMixer(const ReflectionMixer&) = delete;
	ReflectionMixer& operator=(const ReflectionMixer&) = delete;
	ReflectionMixer(ReflectionMixer&& o) noexcept;
	ReflectionMixer& operator=(ReflectionMixer&& o) noexcept;

	bool create(IPLContext context, const IPLAudioSettings& audioSettings,
	            IPLReflectionEffectType type, int irSize, int numChannels);
	void release();
	void reset();

	IPLAudioEffectState apply(const IPLReflectionEffectParams& params, IPLAudioBuffer* out);

	bool isValid() const { return mixer != nullptr; }
	IPLReflectionMixer get() const { return mixer; }

private:
	IPLReflectionMixer mixer = nullptr;
};

// -----------------------------------------------------------------------------
// Path Effect (baked pathing)
// Maps to Steam Audio itest: pathing
// -----------------------------------------------------------------------------
class PathEffect {
public:
	PathEffect() = default;
	~PathEffect();
	PathEffect(const PathEffect&) = delete;
	PathEffect& operator=(const PathEffect&) = delete;
	PathEffect(PathEffect&& o) noexcept;
	PathEffect& operator=(PathEffect&& o) noexcept;

	bool create(IPLContext context, const IPLAudioSettings& audioSettings,
	            int maxOrder, bool spatialize, IPLHRTF hrtf = nullptr,
	            IPLSpeakerLayoutType layout = IPL_SPEAKERLAYOUTTYPE_STEREO);
	void release();
	void reset();

	IPLAudioEffectState apply(const IPLPathEffectParams& params, IPLAudioBuffer* in, IPLAudioBuffer* out);
	IPLAudioEffectState getTail(IPLAudioBuffer* out);
	IPLint32 getTailSize() const;

	bool isValid() const { return effect != nullptr; }
	IPLPathEffect get() const { return effect; }

private:
	IPLPathEffect effect = nullptr;
};

} // namespace ofxSteamAudio
