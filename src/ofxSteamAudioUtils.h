#pragma once

#include "ofMain.h"
#include "phonon.h"
#include <string>

namespace ofxSteamAudio {

/// Convert openFrameworks / glm vector to Steam Audio IPLVector3.
inline IPLVector3 toIPL(const glm::vec3& v) {
	return IPLVector3{ v.x, v.y, v.z };
}

inline IPLVector3 toIPL(const ofVec3f& v) {
	return IPLVector3{ v.x, v.y, v.z };
}

inline glm::vec3 toGlm(const IPLVector3& v) {
	return glm::vec3(v.x, v.y, v.z);
}

inline ofVec3f toOf(const IPLVector3& v) {
	return ofVec3f(v.x, v.y, v.z);
}

/// Human-readable status string for IPLerror codes.
inline const char* errorString(IPLerror err) {
	switch (err) {
		case IPL_STATUS_SUCCESS: return "IPL_STATUS_SUCCESS";
		case IPL_STATUS_FAILURE: return "IPL_STATUS_FAILURE";
		case IPL_STATUS_OUTOFMEMORY: return "IPL_STATUS_OUTOFMEMORY";
		case IPL_STATUS_INITIALIZATION: return "IPL_STATUS_INITIALIZATION";
		default: return "IPL_STATUS_UNKNOWN";
	}
}

inline bool check(IPLerror err, const char* what) {
	if (err != IPL_STATUS_SUCCESS) {
		ofLogError("ofxSteamAudio") << what << " failed: " << errorString(err);
		return false;
	}
	return true;
}

/// Number of Ambisonic channels for a given order: (order+1)^2
inline int ambisonicsChannels(int order) {
	return (order + 1) * (order + 1);
}

/// Steam Audio default material presets (absorption L/M/H, scattering, transmission L/M/H).
namespace Materials {
	inline IPLMaterial make(float a0, float a1, float a2, float sc,
	                        float t0, float t1, float t2) {
		IPLMaterial m{};
		m.absorption[0] = a0; m.absorption[1] = a1; m.absorption[2] = a2;
		m.scattering = sc;
		m.transmission[0] = t0; m.transmission[1] = t1; m.transmission[2] = t2;
		return m;
	}
	inline IPLMaterial generic()  { return make(0.10f,0.20f,0.30f, 0.05f, 0.100f,0.050f,0.030f); }
	inline IPLMaterial brick()    { return make(0.03f,0.04f,0.07f, 0.05f, 0.015f,0.015f,0.015f); }
	inline IPLMaterial concrete() { return make(0.05f,0.07f,0.08f, 0.05f, 0.015f,0.002f,0.001f); }
	inline IPLMaterial carpet()   { return make(0.24f,0.69f,0.73f, 0.05f, 0.020f,0.005f,0.003f); }
	inline IPLMaterial glass()    { return make(0.06f,0.03f,0.02f, 0.05f, 0.060f,0.044f,0.011f); }
	inline IPLMaterial wood()     { return make(0.11f,0.07f,0.06f, 0.05f, 0.070f,0.014f,0.005f); }
	inline IPLMaterial metal()    { return make(0.20f,0.07f,0.06f, 0.05f, 0.200f,0.025f,0.010f); }
	inline IPLMaterial plaster()  { return make(0.12f,0.06f,0.04f, 0.05f, 0.056f,0.056f,0.004f); }
	inline IPLMaterial ceramic()  { return make(0.01f,0.02f,0.02f, 0.05f, 0.060f,0.044f,0.011f); }
	inline IPLMaterial rock()     { return make(0.13f,0.20f,0.24f, 0.05f, 0.015f,0.002f,0.001f); }
}

/// Identity 4x4 matrix for instanced mesh transforms.
inline IPLMatrix4x4 identityMatrix() {
	IPLMatrix4x4 m{};
	m.elements[0][0] = m.elements[1][1] = m.elements[2][2] = m.elements[3][3] = 1.0f;
	return m;
}

/// Build a coordinate space from position + orientation (listener or source).
inline IPLCoordinateSpace3 makeCoordinateSpace(const glm::vec3& origin,
                                               const glm::vec3& ahead = glm::vec3(0, 0, -1),
                                               const glm::vec3& up = glm::vec3(0, 1, 0)) {
	IPLCoordinateSpace3 cs{};
	glm::vec3 a = glm::normalize(ahead);
	glm::vec3 u = glm::normalize(up);
	glm::vec3 r = glm::normalize(glm::cross(a, u));
	u = glm::normalize(glm::cross(r, a));
	cs.origin = toIPL(origin);
	cs.ahead = toIPL(a);
	cs.up = toIPL(u);
	cs.right = toIPL(r);
	return cs;
}

/// Unit direction from listener to source in listener space (wraps iplCalculateRelativeDirection).
inline IPLVector3 relativeDirection(IPLContext context,
                                    const glm::vec3& sourcePos,
                                    const glm::vec3& listenerPos = glm::vec3(0),
                                    const glm::vec3& listenerAhead = glm::vec3(0, 0, -1),
                                    const glm::vec3& listenerUp = glm::vec3(0, 1, 0)) {
	return iplCalculateRelativeDirection(context,
		toIPL(sourcePos), toIPL(listenerPos), toIPL(listenerAhead), toIPL(listenerUp));
}

inline float distanceAttenuation(IPLContext context,
                                 const glm::vec3& source,
                                 const glm::vec3& listener,
                                 IPLDistanceAttenuationModel* model = nullptr) {
	IPLDistanceAttenuationModel defaultModel{};
	defaultModel.type = IPL_DISTANCEATTENUATIONTYPE_DEFAULT;
	if (!model) model = &defaultModel;
	return iplDistanceAttenuationCalculate(context, toIPL(source), toIPL(listener), model);
}

inline void airAbsorption(IPLContext context,
                          const glm::vec3& source,
                          const glm::vec3& listener,
                          IPLfloat32 out[IPL_NUM_BANDS],
                          IPLAirAbsorptionModel* model = nullptr) {
	IPLAirAbsorptionModel defaultModel{};
	defaultModel.type = IPL_AIRABSORPTIONTYPE_DEFAULT;
	if (!model) model = &defaultModel;
	iplAirAbsorptionCalculate(context, toIPL(source), toIPL(listener), model, out);
}

inline float directivity(IPLContext context,
                         const IPLCoordinateSpace3& source,
                         const glm::vec3& listener,
                         IPLDirectivity* model = nullptr) {
	IPLDirectivity defaultModel{};
	defaultModel.dipoleWeight = 0.0f;
	defaultModel.dipolePower = 0.0f;
	if (!model) model = &defaultModel;
	return iplDirectivityCalculate(context, source, toIPL(listener), model);
}

} // namespace ofxSteamAudio
