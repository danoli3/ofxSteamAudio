#pragma once

#include "ofxSteamAudioUtils.h"
#include "ofxSteamAudioScene.h"
#include <vector>

namespace ofxSteamAudio {

// -----------------------------------------------------------------------------
// Energy Field / Impulse Response / Reconstructor
// Maps to Steam Audio itest: energyfield, impulseresponse
// -----------------------------------------------------------------------------
class EnergyField {
public:
	EnergyField() = default;
	~EnergyField();
	EnergyField(const EnergyField&) = delete;
	EnergyField& operator=(const EnergyField&) = delete;
	EnergyField(EnergyField&& o) noexcept;
	EnergyField& operator=(EnergyField&& o) noexcept;

	bool create(IPLContext context, float duration, int order);
	void release();
	void reset();

	IPLint32 numChannels() const;
	IPLint32 numBins() const;
	IPLfloat32* data();
	IPLfloat32* channel(int index);
	IPLfloat32* band(int channelIndex, int bandIndex);

	void copyFrom(EnergyField& src);
	void swapWith(EnergyField& other);
	void add(EnergyField& a, EnergyField& b);
	void scale(EnergyField& in, float scalar);
	void scaleAccum(EnergyField& in, float scalar);

	bool isValid() const { return field != nullptr; }
	IPLEnergyField get() const { return field; }

private:
	IPLEnergyField field = nullptr;
};

class ImpulseResponse {
public:
	ImpulseResponse() = default;
	~ImpulseResponse();
	ImpulseResponse(const ImpulseResponse&) = delete;
	ImpulseResponse& operator=(const ImpulseResponse&) = delete;
	ImpulseResponse(ImpulseResponse&& o) noexcept;
	ImpulseResponse& operator=(ImpulseResponse&& o) noexcept;

	bool create(IPLContext context, float duration, int order, int samplingRate);
	void release();
	void reset();

	IPLint32 numChannels() const;
	IPLint32 numSamples() const;
	IPLfloat32* data();
	IPLfloat32* channel(int index);

	void copyFrom(ImpulseResponse& src);
	void swapWith(ImpulseResponse& other);
	void add(ImpulseResponse& a, ImpulseResponse& b);
	void scale(ImpulseResponse& in, float scalar);
	void scaleAccum(ImpulseResponse& in, float scalar);

	bool isValid() const { return ir != nullptr; }
	IPLImpulseResponse get() const { return ir; }

private:
	IPLImpulseResponse ir = nullptr;
};

class Reconstructor {
public:
	Reconstructor() = default;
	~Reconstructor();
	Reconstructor(const Reconstructor&) = delete;
	Reconstructor& operator=(const Reconstructor&) = delete;
	Reconstructor(Reconstructor&& o) noexcept;
	Reconstructor& operator=(Reconstructor&& o) noexcept;

	bool create(IPLContext context, float maxDuration, int maxOrder, int samplingRate);
	void release();
	void reconstruct(int numInputs, IPLReconstructorInputs* inputs,
	                 IPLReconstructorSharedInputs* sharedInputs, IPLReconstructorOutputs* outputs);

	bool isValid() const { return reconstructor != nullptr; }
	IPLReconstructor get() const { return reconstructor; }

private:
	IPLReconstructor reconstructor = nullptr;
};

// -----------------------------------------------------------------------------
// Probes
// Maps to Steam Audio itest: probes
// -----------------------------------------------------------------------------
class ProbeArray {
public:
	ProbeArray() = default;
	~ProbeArray();
	ProbeArray(const ProbeArray&) = delete;
	ProbeArray& operator=(const ProbeArray&) = delete;
	ProbeArray(ProbeArray&& o) noexcept;
	ProbeArray& operator=(ProbeArray&& o) noexcept;

	bool create(IPLContext context);
	void release();

	void generate(IPLScene scene, const IPLProbeGenerationParams& params);
	IPLint32 numProbes() const;
	IPLSphere getProbe(int index) const;

	bool isValid() const { return array != nullptr; }
	IPLProbeArray get() const { return array; }

private:
	IPLProbeArray array = nullptr;
};

class ProbeBatch {
public:
	ProbeBatch() = default;
	~ProbeBatch();
	ProbeBatch(const ProbeBatch&) = delete;
	ProbeBatch& operator=(const ProbeBatch&) = delete;
	ProbeBatch(ProbeBatch&& o) noexcept;
	ProbeBatch& operator=(ProbeBatch&& o) noexcept;

	bool create(IPLContext context);
	bool load(IPLContext context, IPLSerializedObject serialized);
	void release();

	void save(IPLSerializedObject serialized) const;
	IPLint32 numProbes() const;
	void addProbe(const IPLSphere& probe);
	void addProbeArray(IPLProbeArray probeArray);
	void removeProbe(int index);
	void commit();
	void removeData(IPLBakedDataIdentifier* identifier);
	IPLsize getDataSize(IPLBakedDataIdentifier* identifier) const;
	void getEnergyField(IPLBakedDataIdentifier* identifier, int probeIndex, IPLEnergyField energyField);
	void getReverb(IPLBakedDataIdentifier* identifier, int probeIndex, IPLfloat32* reverbTimes);

	bool isValid() const { return batch != nullptr; }
	IPLProbeBatch get() const { return batch; }

private:
	IPLProbeBatch batch = nullptr;
};

// -----------------------------------------------------------------------------
// Bakers (static helpers wrapping global bake APIs)
// -----------------------------------------------------------------------------
struct Bakers {
	static void bakeReflections(IPLContext context, IPLReflectionsBakeParams* params,
	                            IPLProgressCallback cb = nullptr, void* userData = nullptr) {
		iplReflectionsBakerBake(context, params, cb, userData);
	}
	static void cancelReflectionsBake(IPLContext context) {
		iplReflectionsBakerCancelBake(context);
	}
	static void bakePathing(IPLContext context, IPLPathBakeParams* params,
	                        IPLProgressCallback cb = nullptr, void* userData = nullptr) {
		iplPathBakerBake(context, params, cb, userData);
	}
	static void cancelPathingBake(IPLContext context) {
		iplPathBakerCancelBake(context);
	}
};

// -----------------------------------------------------------------------------
// Simulation Source
// -----------------------------------------------------------------------------
class SimSource {
public:
	SimSource() = default;
	~SimSource();
	SimSource(const SimSource&) = delete;
	SimSource& operator=(const SimSource&) = delete;
	SimSource(SimSource&& o) noexcept;
	SimSource& operator=(SimSource&& o) noexcept;

	bool create(IPLSimulator simulator, IPLSimulationFlags flags = IPL_SIMULATIONFLAGS_DIRECT);
	void release();

	void add(IPLSimulator simulator);
	void remove(IPLSimulator simulator);
	void setInputs(IPLSimulationFlags flags, IPLSimulationInputs* inputs);
	void getOutputs(IPLSimulationFlags flags, IPLSimulationOutputs* outputs);

	bool isValid() const { return source != nullptr; }
	IPLSource get() const { return source; }

private:
	IPLSource source = nullptr;
};

// -----------------------------------------------------------------------------
// Simulator
// Maps to Steam Audio itest: directsimulator, reflectionsimulator, pathing
// -----------------------------------------------------------------------------
class Simulator {
public:
	Simulator() = default;
	~Simulator();
	Simulator(const Simulator&) = delete;
	Simulator& operator=(const Simulator&) = delete;
	Simulator(Simulator&& o) noexcept;
	Simulator& operator=(Simulator&& o) noexcept;

	/// Create with sensible defaults for direct + optional reflections.
	bool create(IPLContext context, const IPLAudioSettings& audioSettings,
	            IPLSimulationFlags flags = IPL_SIMULATIONFLAGS_DIRECT,
	            IPLSceneType sceneType = IPL_SCENETYPE_DEFAULT,
	            IPLReflectionEffectType reflectionType = IPL_REFLECTIONEFFECTTYPE_PARAMETRIC,
	            int maxNumSources = 8);
	bool create(IPLContext context, const IPLSimulationSettings& settings);
	void release();

	void setScene(IPLScene scene);
	void addProbeBatch(IPLProbeBatch batch);
	void removeProbeBatch(IPLProbeBatch batch);
	void setSharedInputs(IPLSimulationFlags flags, IPLSimulationSharedInputs* sharedInputs);
	void commit();

	void runDirect();
	void runReflections();
	void runPathing();

	bool isValid() const { return simulator != nullptr; }
	IPLSimulator get() const { return simulator; }

private:
	IPLSimulator simulator = nullptr;
};

} // namespace ofxSteamAudio
