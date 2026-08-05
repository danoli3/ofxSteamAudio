#include "ofxSteamAudioContext.h"

namespace ofxSteamAudio {

std::atomic<size_t> Context::totalAllocated{0};
std::atomic<size_t> Context::peakUsage{0};

Context::~Context() {
	release();
}

Context::Context(Context&& other) noexcept
	: context(other.context), memoryTracking(other.memoryTracking) {
	other.context = nullptr;
}

Context& Context::operator=(Context&& other) noexcept {
	if (this != &other) {
		release();
		context = other.context;
		memoryTracking = other.memoryTracking;
		other.context = nullptr;
	}
	return *this;
}

void* IPLCALL Context::allocate(IPLsize size, IPLsize alignment) {
	void* ptr = nullptr;
#if defined(_WIN32)
	ptr = _aligned_malloc(size, alignment);
#else
	if (posix_memalign(&ptr, alignment, size) != 0) ptr = nullptr;
#endif
	if (ptr) {
		size_t newTotal = totalAllocated.fetch_add(size) + size;
		size_t currentPeak = peakUsage.load();
		while (newTotal > currentPeak) {
			if (peakUsage.compare_exchange_weak(currentPeak, newTotal)) break;
			currentPeak = peakUsage.load();
		}
	}
	return ptr;
}

void IPLCALL Context::free(void* block) {
	if (!block) return;
#if defined(_WIN32)
	_aligned_free(block);
#else
	::free(block);
#endif
}

bool Context::setup(bool trackMemory, IPLSIMDLevel simdLevel, IPLContextFlags flags) {
	release();
	memoryTracking = trackMemory;

	IPLContextSettings settings{};
	settings.version = STEAMAUDIO_VERSION;
	settings.simdLevel = simdLevel;
	settings.flags = flags;
	settings.logCallback = [](IPLLogLevel level, const char* msg) {
		switch (level) {
			case IPL_LOGLEVEL_ERROR:   ofLogError("SteamAudio") << msg; break;
			case IPL_LOGLEVEL_WARNING: ofLogWarning("SteamAudio") << msg; break;
			default:                   ofLogNotice("SteamAudio") << msg; break;
		}
	};
	if (memoryTracking) {
		settings.allocateCallback = allocate;
		settings.freeCallback = free;
	}

	return check(iplContextCreate(&settings, &context), "iplContextCreate");
}

void Context::release() {
	if (context) {
		iplContextRelease(&context);
		context = nullptr;
	}
}

void Context::printMemoryUsage() const {
	if (!memoryTracking) return;
	ofLogNotice("ofxSteamAudio") << "Memory Total: " << (totalAllocated / 1024)
		<< " KB | Peak: " << (peakUsage / 1024) << " KB";
}

} // namespace ofxSteamAudio
