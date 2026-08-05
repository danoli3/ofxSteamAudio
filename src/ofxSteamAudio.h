#pragma once

/**
 * ofxSteamAudio — openFrameworks wrapper for Steam Audio 4.8.1 (Phonon C API)
 *
 * Modular RAII wrappers covering the public Steam Audio C API:
 *   Context, AudioBuffer, HRTF
 *   Effects: Panning, Binaural, VirtualSurround, Ambisonics*, Direct, Reflection, Path
 *   Scene, StaticMesh, InstancedMesh, Embree, Serialization
 *   Simulator, SimSource, Probes, Bakers, EnergyField, ImpulseResponse, Reconstructor
 *   High-level Engine for multi-source binaural rendering
 *
 * Include this header for the full API. Individual modules may also be included.
 */

#include "ofxSteamAudioUtils.h"
#include "ofxSteamAudioContext.h"
#include "ofxSteamAudioBuffer.h"
#include "ofxSteamAudioHRTF.h"
#include "ofxSteamAudioEffects.h"
#include "ofxSteamAudioScene.h"
#include "ofxSteamAudioSimulator.h"

namespace ofxSteamAudio {

/// High-level multi-source binaural spatializer (HRTF).
/// Suitable for most interactive demos; for advanced sim use Simulator + effects directly.
class Engine {
public:
	Engine() = default;
	~Engine();

	bool setup(int sampleRate = 44100, int bufferSize = 512, bool trackMemory = false);
	void shutdown();

	int getSampleRate() const { return sampleRate; }
	int getBufferSize() const { return bufferSize; }
	bool isReady() const { return ready; }

	IPLContext getContext() const { return context.get(); }
	IPLHRTF getHRTF() const { return hrtf.get(); }
	const IPLAudioSettings& getAudioSettings() const { return audioSettings; }

	/// Listener pose (world space). Steam Audio: +Y up, -Z ahead by default.
	void setListener(const glm::vec3& position,
	                 const glm::vec3& ahead = glm::vec3(0, 0, -1),
	                 const glm::vec3& up = glm::vec3(0, 1, 0));
	void setListener(const ofCamera& cam);
	void setListener(const ofNode& node);

	int addSource(const glm::vec3& position = glm::vec3(0));
	void updateSource(int sourceId, const glm::vec3& position);
	void removeSource(int sourceId);
	void setSourceGain(int sourceId, float gain);
	void setSourceFrequency(int sourceId, float hz);

	/// Process one audio frame into an interleaved stereo ofSoundBuffer (clears then fills).
	void processAudio(ofSoundBuffer& outputBuffer);

	/// Spatialize a mono deinterleaved buffer for one source into stereo mix.
	void processSource(int sourceId, IPLAudioBuffer* monoIn, ofSoundBuffer& outputBuffer, float mixGain = 1.0f);

	size_t getTotalMemoryUsed() const { return Context::getTotalAllocated(); }
	size_t getPeakMemoryUsed() const { return Context::getPeakAllocated(); }
	void printMemoryUsage() const { context.printMemoryUsage(); }

private:
	Context context;
	HRTF hrtf;
	BinauralEffect binaural;
	AudioBuffer tempIn;
	AudioBuffer tempOut;

	IPLAudioSettings audioSettings{};
	int sampleRate = 44100;
	int bufferSize = 512;
	bool ready = false;

	glm::vec3 listenerPos{0};
	glm::vec3 listenerAhead{0, 0, -1};
	glm::vec3 listenerUp{0, 1, 0};

	struct Source {
		glm::vec3 position{0};
		float phase = 0.0f;
		float frequency = 220.0f;
		float gain = 0.5f;
		bool active = true;
	};
	std::vector<Source> sources;
};

} // namespace ofxSteamAudio
