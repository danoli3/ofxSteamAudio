#pragma once

#include "ofxSteamAudioUtils.h"
#include <vector>

namespace ofxSteamAudio {

/// RAII wrapper for IPLSerializedObject.
class SerializedObject {
public:
	SerializedObject() = default;
	~SerializedObject();
	SerializedObject(const SerializedObject&) = delete;
	SerializedObject& operator=(const SerializedObject&) = delete;
	SerializedObject(SerializedObject&& o) noexcept;
	SerializedObject& operator=(SerializedObject&& o) noexcept;

	bool create(IPLContext context, IPLbyte* data = nullptr, IPLsize size = 0);
	void release();

	IPLsize getSize() const;
	IPLbyte* getData() const;

	bool isValid() const { return obj != nullptr; }
	IPLSerializedObject get() const { return obj; }

private:
	IPLSerializedObject obj = nullptr;
};

/// Optional Embree device (x86_64 only on macOS; arm64 uses default ray tracer).
class EmbreeDevice {
public:
	EmbreeDevice() = default;
	~EmbreeDevice();
	EmbreeDevice(const EmbreeDevice&) = delete;
	EmbreeDevice& operator=(const EmbreeDevice&) = delete;
	EmbreeDevice(EmbreeDevice&& o) noexcept;
	EmbreeDevice& operator=(EmbreeDevice&& o) noexcept;

	bool create(IPLContext context);
	void release();

	bool isValid() const { return device != nullptr; }
	IPLEmbreeDevice get() const { return device; }

private:
	IPLEmbreeDevice device = nullptr;
};

/// RAII wrapper for IPLScene + static/instanced meshes.
/// Maps to Steam Audio itest: scene, staticmesh, instancedmesh, raytracer
class Scene {
public:
	Scene() = default;
	~Scene();
	Scene(const Scene&) = delete;
	Scene& operator=(const Scene&) = delete;
	Scene(Scene&& o) noexcept;
	Scene& operator=(Scene&& o) noexcept;

	bool create(IPLContext context, IPLSceneType type = IPL_SCENETYPE_DEFAULT,
	            IPLEmbreeDevice embree = nullptr);
	bool load(IPLContext context, IPLSerializedObject serialized,
	          IPLSceneType type = IPL_SCENETYPE_DEFAULT, IPLEmbreeDevice embree = nullptr);
	void release();

	void commit();
	void save(IPLSerializedObject serialized) const;
	void saveOBJ(const std::string& fileBaseName) const;

	/// Create and add a static mesh; returns mesh handle (owned by this Scene).
	IPLStaticMesh addStaticMesh(const IPLStaticMeshSettings& settings);
	/// Convenience: axis-aligned box centered at origin (or offset).
	IPLStaticMesh addBox(float width, float height, float depth,
	                     const IPLMaterial& material = Materials::generic(),
	                     const glm::vec3& center = glm::vec3(0));
	/// Convenience: ground plane (XZ) as two triangles.
	IPLStaticMesh addGroundPlane(float halfExtent = 50.0f,
	                             const IPLMaterial& material = Materials::concrete());

	void removeStaticMesh(IPLStaticMesh mesh);
	void setMaterial(IPLStaticMesh mesh, IPLMaterial* material, int index);

	IPLInstancedMesh addInstancedMesh(IPLScene subScene, const IPLMatrix4x4& transform);
	void removeInstancedMesh(IPLInstancedMesh mesh);
	void updateInstancedMeshTransform(IPLInstancedMesh mesh, const IPLMatrix4x4& transform);

	bool isValid() const { return scene != nullptr; }
	IPLScene get() const { return scene; }
	operator IPLScene() const { return scene; }

private:
	IPLContext ctx = nullptr;
	IPLScene scene = nullptr;
	std::vector<IPLStaticMesh> staticMeshes;
	std::vector<IPLInstancedMesh> instancedMeshes;
};

} // namespace ofxSteamAudio
