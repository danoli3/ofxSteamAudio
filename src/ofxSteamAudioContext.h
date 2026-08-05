#pragma once

#include "ofxSteamAudioUtils.h"
#include <atomic>

namespace ofxSteamAudio {

/// RAII wrapper for IPLContext (must be created before any other Steam Audio object).
class Context {
public:
	Context() = default;
	~Context();

	Context(const Context&) = delete;
	Context& operator=(const Context&) = delete;
	Context(Context&& other) noexcept;
	Context& operator=(Context&& other) noexcept;

	/// Create context. Optional custom allocators for memory tracking.
	bool setup(bool trackMemory = false,
	           IPLSIMDLevel simdLevel = IPL_SIMDLEVEL_AVX2,
	           IPLContextFlags flags = static_cast<IPLContextFlags>(0));

	void release();

	bool isValid() const { return context != nullptr; }
	IPLContext get() const { return context; }
	operator IPLContext() const { return context; }

	static size_t getTotalAllocated() { return totalAllocated.load(); }
	static size_t getPeakAllocated() { return peakUsage.load(); }
	void printMemoryUsage() const;

private:
	IPLContext context = nullptr;
	bool memoryTracking = false;

	static std::atomic<size_t> totalAllocated;
	static std::atomic<size_t> peakUsage;
	static void* IPLCALL allocate(IPLsize size, IPLsize alignment);
	static void IPLCALL free(void* block);
};

} // namespace ofxSteamAudio
