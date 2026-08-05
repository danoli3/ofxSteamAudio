#include "ofxSteamAudioSimulator.h"

namespace ofxSteamAudio {

// =============================================================================
// EnergyField
// =============================================================================
EnergyField::~EnergyField() { release(); }
EnergyField::EnergyField(EnergyField&& o) noexcept : field(o.field) { o.field = nullptr; }
EnergyField& EnergyField::operator=(EnergyField&& o) noexcept {
	if (this != &o) { release(); field = o.field; o.field = nullptr; }
	return *this;
}
bool EnergyField::create(IPLContext context, float duration, int order) {
	release();
	IPLEnergyFieldSettings s{};
	s.duration = duration;
	s.order = order;
	return check(iplEnergyFieldCreate(context, &s, &field), "iplEnergyFieldCreate");
}
void EnergyField::release() { if (field) { iplEnergyFieldRelease(&field); field = nullptr; } }
void EnergyField::reset() { if (field) iplEnergyFieldReset(field); }
IPLint32 EnergyField::numChannels() const { return field ? iplEnergyFieldGetNumChannels(field) : 0; }
IPLint32 EnergyField::numBins() const { return field ? iplEnergyFieldGetNumBins(field) : 0; }
IPLfloat32* EnergyField::data() { return field ? iplEnergyFieldGetData(field) : nullptr; }
IPLfloat32* EnergyField::channel(int index) { return field ? iplEnergyFieldGetChannel(field, index) : nullptr; }
IPLfloat32* EnergyField::band(int channelIndex, int bandIndex) {
	return field ? iplEnergyFieldGetBand(field, channelIndex, bandIndex) : nullptr;
}
void EnergyField::copyFrom(EnergyField& src) { if (field && src.field) iplEnergyFieldCopy(src.field, field); }
void EnergyField::swapWith(EnergyField& other) { if (field && other.field) iplEnergyFieldSwap(field, other.field); }
void EnergyField::add(EnergyField& a, EnergyField& b) {
	if (field && a.field && b.field) iplEnergyFieldAdd(a.field, b.field, field);
}
void EnergyField::scale(EnergyField& in, float scalar) {
	if (field && in.field) iplEnergyFieldScale(in.field, scalar, field);
}
void EnergyField::scaleAccum(EnergyField& in, float scalar) {
	if (field && in.field) iplEnergyFieldScaleAccum(in.field, scalar, field);
}

// =============================================================================
// ImpulseResponse
// =============================================================================
ImpulseResponse::~ImpulseResponse() { release(); }
ImpulseResponse::ImpulseResponse(ImpulseResponse&& o) noexcept : ir(o.ir) { o.ir = nullptr; }
ImpulseResponse& ImpulseResponse::operator=(ImpulseResponse&& o) noexcept {
	if (this != &o) { release(); ir = o.ir; o.ir = nullptr; }
	return *this;
}
bool ImpulseResponse::create(IPLContext context, float duration, int order, int samplingRate) {
	release();
	IPLImpulseResponseSettings s{};
	s.duration = duration;
	s.order = order;
	s.samplingRate = samplingRate;
	return check(iplImpulseResponseCreate(context, &s, &ir), "iplImpulseResponseCreate");
}
void ImpulseResponse::release() { if (ir) { iplImpulseResponseRelease(&ir); ir = nullptr; } }
void ImpulseResponse::reset() { if (ir) iplImpulseResponseReset(ir); }
IPLint32 ImpulseResponse::numChannels() const { return ir ? iplImpulseResponseGetNumChannels(ir) : 0; }
IPLint32 ImpulseResponse::numSamples() const { return ir ? iplImpulseResponseGetNumSamples(ir) : 0; }
IPLfloat32* ImpulseResponse::data() { return ir ? iplImpulseResponseGetData(ir) : nullptr; }
IPLfloat32* ImpulseResponse::channel(int index) { return ir ? iplImpulseResponseGetChannel(ir, index) : nullptr; }
void ImpulseResponse::copyFrom(ImpulseResponse& src) { if (ir && src.ir) iplImpulseResponseCopy(src.ir, ir); }
void ImpulseResponse::swapWith(ImpulseResponse& other) { if (ir && other.ir) iplImpulseResponseSwap(ir, other.ir); }
void ImpulseResponse::add(ImpulseResponse& a, ImpulseResponse& b) {
	if (ir && a.ir && b.ir) iplImpulseResponseAdd(a.ir, b.ir, ir);
}
void ImpulseResponse::scale(ImpulseResponse& in, float scalar) {
	if (ir && in.ir) iplImpulseResponseScale(in.ir, scalar, ir);
}
void ImpulseResponse::scaleAccum(ImpulseResponse& in, float scalar) {
	if (ir && in.ir) iplImpulseResponseScaleAccum(in.ir, scalar, ir);
}

// =============================================================================
// Reconstructor
// =============================================================================
Reconstructor::~Reconstructor() { release(); }
Reconstructor::Reconstructor(Reconstructor&& o) noexcept : reconstructor(o.reconstructor) { o.reconstructor = nullptr; }
Reconstructor& Reconstructor::operator=(Reconstructor&& o) noexcept {
	if (this != &o) { release(); reconstructor = o.reconstructor; o.reconstructor = nullptr; }
	return *this;
}
bool Reconstructor::create(IPLContext context, float maxDuration, int maxOrder, int samplingRate) {
	release();
	IPLReconstructorSettings s{};
	s.maxDuration = maxDuration;
	s.maxOrder = maxOrder;
	s.samplingRate = samplingRate;
	return check(iplReconstructorCreate(context, &s, &reconstructor), "iplReconstructorCreate");
}
void Reconstructor::release() {
	if (reconstructor) { iplReconstructorRelease(&reconstructor); reconstructor = nullptr; }
}
void Reconstructor::reconstruct(int numInputs, IPLReconstructorInputs* inputs,
                                IPLReconstructorSharedInputs* sharedInputs, IPLReconstructorOutputs* outputs) {
	if (reconstructor) iplReconstructorReconstruct(reconstructor, numInputs, inputs, sharedInputs, outputs);
}

// =============================================================================
// ProbeArray
// =============================================================================
ProbeArray::~ProbeArray() { release(); }
ProbeArray::ProbeArray(ProbeArray&& o) noexcept : array(o.array) { o.array = nullptr; }
ProbeArray& ProbeArray::operator=(ProbeArray&& o) noexcept {
	if (this != &o) { release(); array = o.array; o.array = nullptr; }
	return *this;
}
bool ProbeArray::create(IPLContext context) {
	release();
	return check(iplProbeArrayCreate(context, &array), "iplProbeArrayCreate");
}
void ProbeArray::release() { if (array) { iplProbeArrayRelease(&array); array = nullptr; } }
void ProbeArray::generate(IPLScene scene, const IPLProbeGenerationParams& params) {
	if (!array) return;
	IPLProbeGenerationParams p = params;
	iplProbeArrayGenerateProbes(array, scene, &p);
}
IPLint32 ProbeArray::numProbes() const { return array ? iplProbeArrayGetNumProbes(array) : 0; }
IPLSphere ProbeArray::getProbe(int index) const {
	return array ? iplProbeArrayGetProbe(array, index) : IPLSphere{};
}

// =============================================================================
// ProbeBatch
// =============================================================================
ProbeBatch::~ProbeBatch() { release(); }
ProbeBatch::ProbeBatch(ProbeBatch&& o) noexcept : batch(o.batch) { o.batch = nullptr; }
ProbeBatch& ProbeBatch::operator=(ProbeBatch&& o) noexcept {
	if (this != &o) { release(); batch = o.batch; o.batch = nullptr; }
	return *this;
}
bool ProbeBatch::create(IPLContext context) {
	release();
	return check(iplProbeBatchCreate(context, &batch), "iplProbeBatchCreate");
}
bool ProbeBatch::load(IPLContext context, IPLSerializedObject serialized) {
	release();
	return check(iplProbeBatchLoad(context, serialized, &batch), "iplProbeBatchLoad");
}
void ProbeBatch::release() { if (batch) { iplProbeBatchRelease(&batch); batch = nullptr; } }
void ProbeBatch::save(IPLSerializedObject serialized) const {
	if (batch) iplProbeBatchSave(batch, serialized);
}
IPLint32 ProbeBatch::numProbes() const { return batch ? iplProbeBatchGetNumProbes(batch) : 0; }
void ProbeBatch::addProbe(const IPLSphere& probe) { if (batch) iplProbeBatchAddProbe(batch, probe); }
void ProbeBatch::addProbeArray(IPLProbeArray probeArray) {
	if (batch) iplProbeBatchAddProbeArray(batch, probeArray);
}
void ProbeBatch::removeProbe(int index) { if (batch) iplProbeBatchRemoveProbe(batch, index); }
void ProbeBatch::commit() { if (batch) iplProbeBatchCommit(batch); }
void ProbeBatch::removeData(IPLBakedDataIdentifier* identifier) {
	if (batch) iplProbeBatchRemoveData(batch, identifier);
}
IPLsize ProbeBatch::getDataSize(IPLBakedDataIdentifier* identifier) const {
	return batch ? iplProbeBatchGetDataSize(batch, identifier) : 0;
}
void ProbeBatch::getEnergyField(IPLBakedDataIdentifier* identifier, int probeIndex, IPLEnergyField energyField) {
	if (batch) iplProbeBatchGetEnergyField(batch, identifier, probeIndex, energyField);
}
void ProbeBatch::getReverb(IPLBakedDataIdentifier* identifier, int probeIndex, IPLfloat32* reverbTimes) {
	if (batch) iplProbeBatchGetReverb(batch, identifier, probeIndex, reverbTimes);
}

// =============================================================================
// SimSource
// =============================================================================
SimSource::~SimSource() { release(); }
SimSource::SimSource(SimSource&& o) noexcept : source(o.source) { o.source = nullptr; }
SimSource& SimSource::operator=(SimSource&& o) noexcept {
	if (this != &o) { release(); source = o.source; o.source = nullptr; }
	return *this;
}
bool SimSource::create(IPLSimulator simulator, IPLSimulationFlags flags) {
	release();
	IPLSourceSettings s{};
	s.flags = flags;
	return check(iplSourceCreate(simulator, &s, &source), "iplSourceCreate");
}
void SimSource::release() { if (source) { iplSourceRelease(&source); source = nullptr; } }
void SimSource::add(IPLSimulator simulator) { if (source) iplSourceAdd(source, simulator); }
void SimSource::remove(IPLSimulator simulator) { if (source) iplSourceRemove(source, simulator); }
void SimSource::setInputs(IPLSimulationFlags flags, IPLSimulationInputs* inputs) {
	if (source) iplSourceSetInputs(source, flags, inputs);
}
void SimSource::getOutputs(IPLSimulationFlags flags, IPLSimulationOutputs* outputs) {
	if (source) iplSourceGetOutputs(source, flags, outputs);
}

// =============================================================================
// Simulator
// =============================================================================
Simulator::~Simulator() { release(); }
Simulator::Simulator(Simulator&& o) noexcept : simulator(o.simulator) { o.simulator = nullptr; }
Simulator& Simulator::operator=(Simulator&& o) noexcept {
	if (this != &o) { release(); simulator = o.simulator; o.simulator = nullptr; }
	return *this;
}

bool Simulator::create(IPLContext context, const IPLAudioSettings& audioSettings,
                       IPLSimulationFlags flags, IPLSceneType sceneType,
                       IPLReflectionEffectType reflectionType, int maxNumSources) {
	IPLSimulationSettings s{};
	s.flags = flags;
	s.sceneType = sceneType;
	s.reflectionType = reflectionType;
	s.maxNumOcclusionSamples = 16;
	s.maxNumRays = 4096;
	s.numDiffuseSamples = 32;
	s.maxDuration = 2.0f;
	s.maxOrder = 1;
	s.maxNumSources = maxNumSources;
	s.numThreads = 1;
	s.rayBatchSize = 1;
	s.numVisSamples = 4;
	s.samplingRate = audioSettings.samplingRate;
	s.frameSize = audioSettings.frameSize;
	return create(context, s);
}

bool Simulator::create(IPLContext context, const IPLSimulationSettings& settings) {
	release();
	IPLSimulationSettings s = settings;
	return check(iplSimulatorCreate(context, &s, &simulator), "iplSimulatorCreate");
}

void Simulator::release() {
	if (simulator) {
		iplSimulatorRelease(&simulator);
		simulator = nullptr;
	}
}

void Simulator::setScene(IPLScene scene) { if (simulator) iplSimulatorSetScene(simulator, scene); }
void Simulator::addProbeBatch(IPLProbeBatch batch) {
	if (simulator) iplSimulatorAddProbeBatch(simulator, batch);
}
void Simulator::removeProbeBatch(IPLProbeBatch batch) {
	if (simulator) iplSimulatorRemoveProbeBatch(simulator, batch);
}
void Simulator::setSharedInputs(IPLSimulationFlags flags, IPLSimulationSharedInputs* sharedInputs) {
	if (simulator) iplSimulatorSetSharedInputs(simulator, flags, sharedInputs);
}
void Simulator::commit() { if (simulator) iplSimulatorCommit(simulator); }
void Simulator::runDirect() { if (simulator) iplSimulatorRunDirect(simulator); }
void Simulator::runReflections() { if (simulator) iplSimulatorRunReflections(simulator); }
void Simulator::runPathing() { if (simulator) iplSimulatorRunPathing(simulator); }

} // namespace ofxSteamAudio
