#include "ofxSteamAudioScene.h"

namespace ofxSteamAudio {

// =============================================================================
// SerializedObject
// =============================================================================
SerializedObject::~SerializedObject() { release(); }
SerializedObject::SerializedObject(SerializedObject&& o) noexcept : obj(o.obj) { o.obj = nullptr; }
SerializedObject& SerializedObject::operator=(SerializedObject&& o) noexcept {
	if (this != &o) { release(); obj = o.obj; o.obj = nullptr; }
	return *this;
}

bool SerializedObject::create(IPLContext context, IPLbyte* data, IPLsize size) {
	release();
	IPLSerializedObjectSettings s{};
	s.data = data;
	s.size = size;
	return check(iplSerializedObjectCreate(context, &s, &obj), "iplSerializedObjectCreate");
}
void SerializedObject::release() { if (obj) { iplSerializedObjectRelease(&obj); obj = nullptr; } }
IPLsize SerializedObject::getSize() const { return obj ? iplSerializedObjectGetSize(obj) : 0; }
IPLbyte* SerializedObject::getData() const { return obj ? iplSerializedObjectGetData(obj) : nullptr; }

// =============================================================================
// EmbreeDevice
// =============================================================================
EmbreeDevice::~EmbreeDevice() { release(); }
EmbreeDevice::EmbreeDevice(EmbreeDevice&& o) noexcept : device(o.device) { o.device = nullptr; }
EmbreeDevice& EmbreeDevice::operator=(EmbreeDevice&& o) noexcept {
	if (this != &o) { release(); device = o.device; o.device = nullptr; }
	return *this;
}

bool EmbreeDevice::create(IPLContext context) {
	release();
	IPLEmbreeDeviceSettings s{};
	return check(iplEmbreeDeviceCreate(context, &s, &device), "iplEmbreeDeviceCreate");
}
void EmbreeDevice::release() { if (device) { iplEmbreeDeviceRelease(&device); device = nullptr; } }

// =============================================================================
// Scene
// =============================================================================
Scene::~Scene() { release(); }
Scene::Scene(Scene&& o) noexcept
	: ctx(o.ctx), scene(o.scene),
	  staticMeshes(std::move(o.staticMeshes)),
	  instancedMeshes(std::move(o.instancedMeshes)) {
	o.ctx = nullptr;
	o.scene = nullptr;
}
Scene& Scene::operator=(Scene&& o) noexcept {
	if (this != &o) {
		release();
		ctx = o.ctx;
		scene = o.scene;
		staticMeshes = std::move(o.staticMeshes);
		instancedMeshes = std::move(o.instancedMeshes);
		o.ctx = nullptr;
		o.scene = nullptr;
	}
	return *this;
}

bool Scene::create(IPLContext context, IPLSceneType type, IPLEmbreeDevice embree) {
	release();
	ctx = context;
	IPLSceneSettings s{};
	s.type = type;
	s.embreeDevice = embree;
	return check(iplSceneCreate(context, &s, &scene), "iplSceneCreate");
}

bool Scene::load(IPLContext context, IPLSerializedObject serialized, IPLSceneType type, IPLEmbreeDevice embree) {
	release();
	ctx = context;
	IPLSceneSettings s{};
	s.type = type;
	s.embreeDevice = embree;
	return check(iplSceneLoad(context, &s, serialized, nullptr, nullptr, &scene), "iplSceneLoad");
}

void Scene::release() {
	for (auto& m : instancedMeshes) {
		if (m && scene) iplInstancedMeshRemove(m, scene);
		if (m) iplInstancedMeshRelease(&m);
	}
	instancedMeshes.clear();
	for (auto& m : staticMeshes) {
		if (m && scene) iplStaticMeshRemove(m, scene);
		if (m) iplStaticMeshRelease(&m);
	}
	staticMeshes.clear();
	if (scene) {
		iplSceneRelease(&scene);
		scene = nullptr;
	}
	ctx = nullptr;
}

void Scene::commit() {
	if (scene) iplSceneCommit(scene);
}

void Scene::save(IPLSerializedObject serialized) const {
	if (scene) iplSceneSave(scene, serialized);
}

void Scene::saveOBJ(const std::string& fileBaseName) const {
	if (scene) iplSceneSaveOBJ(scene, fileBaseName.c_str());
}

IPLStaticMesh Scene::addStaticMesh(const IPLStaticMeshSettings& settings) {
	if (!scene) return nullptr;
	IPLStaticMesh mesh = nullptr;
	IPLStaticMeshSettings s = settings;
	if (!check(iplStaticMeshCreate(scene, &s, &mesh), "iplStaticMeshCreate")) return nullptr;
	iplStaticMeshAdd(mesh, scene);
	staticMeshes.push_back(mesh);
	return mesh;
}

IPLStaticMesh Scene::addBox(float width, float height, float depth,
                            const IPLMaterial& material, const glm::vec3& center) {
	const float hx = width * 0.5f, hy = height * 0.5f, hz = depth * 0.5f;
	std::vector<IPLVector3> verts = {
		{center.x - hx, center.y - hy, center.z - hz},
		{center.x + hx, center.y - hy, center.z - hz},
		{center.x + hx, center.y + hy, center.z - hz},
		{center.x - hx, center.y + hy, center.z - hz},
		{center.x - hx, center.y - hy, center.z + hz},
		{center.x + hx, center.y - hy, center.z + hz},
		{center.x + hx, center.y + hy, center.z + hz},
		{center.x - hx, center.y + hy, center.z + hz},
	};
	// CCW winding when looking along outward normal
	std::vector<IPLTriangle> tris = {
		{{0, 2, 1}}, {{0, 3, 2}}, // -Z
		{{4, 5, 6}}, {{4, 6, 7}}, // +Z
		{{0, 1, 5}}, {{0, 5, 4}}, // -Y
		{{3, 6, 2}}, {{3, 7, 6}}, // +Y
		{{0, 4, 7}}, {{0, 7, 3}}, // -X
		{{1, 2, 6}}, {{1, 6, 5}}, // +X
	};
	std::vector<IPLint32> matIndices(tris.size(), 0);
	IPLMaterial mats[1] = { material };

	IPLStaticMeshSettings s{};
	s.numVertices = (IPLint32)verts.size();
	s.numTriangles = (IPLint32)tris.size();
	s.numMaterials = 1;
	s.vertices = verts.data();
	s.triangles = tris.data();
	s.materialIndices = matIndices.data();
	s.materials = mats;
	return addStaticMesh(s);
}

IPLStaticMesh Scene::addGroundPlane(float halfExtent, const IPLMaterial& material) {
	std::vector<IPLVector3> verts = {
		{-halfExtent, 0, -halfExtent},
		{ halfExtent, 0, -halfExtent},
		{ halfExtent, 0,  halfExtent},
		{-halfExtent, 0,  halfExtent},
	};
	std::vector<IPLTriangle> tris = {
		{{0, 1, 2}},
		{{0, 2, 3}},
	};
	std::vector<IPLint32> matIndices(2, 0);
	IPLMaterial mats[1] = { material };

	IPLStaticMeshSettings s{};
	s.numVertices = 4;
	s.numTriangles = 2;
	s.numMaterials = 1;
	s.vertices = verts.data();
	s.triangles = tris.data();
	s.materialIndices = matIndices.data();
	s.materials = mats;
	return addStaticMesh(s);
}

void Scene::removeStaticMesh(IPLStaticMesh mesh) {
	if (!mesh || !scene) return;
	iplStaticMeshRemove(mesh, scene);
	for (auto it = staticMeshes.begin(); it != staticMeshes.end(); ++it) {
		if (*it == mesh) {
			iplStaticMeshRelease(&mesh);
			staticMeshes.erase(it);
			return;
		}
	}
}

void Scene::setMaterial(IPLStaticMesh mesh, IPLMaterial* material, int index) {
	if (mesh && scene) iplStaticMeshSetMaterial(mesh, scene, material, index);
}

IPLInstancedMesh Scene::addInstancedMesh(IPLScene subScene, const IPLMatrix4x4& transform) {
	if (!scene) return nullptr;
	IPLInstancedMesh mesh = nullptr;
	IPLInstancedMeshSettings s{};
	s.subScene = subScene;
	s.transform = transform;
	if (!check(iplInstancedMeshCreate(scene, &s, &mesh), "iplInstancedMeshCreate")) return nullptr;
	iplInstancedMeshAdd(mesh, scene);
	instancedMeshes.push_back(mesh);
	return mesh;
}

void Scene::removeInstancedMesh(IPLInstancedMesh mesh) {
	if (!mesh || !scene) return;
	iplInstancedMeshRemove(mesh, scene);
	for (auto it = instancedMeshes.begin(); it != instancedMeshes.end(); ++it) {
		if (*it == mesh) {
			iplInstancedMeshRelease(&mesh);
			instancedMeshes.erase(it);
			return;
		}
	}
}

void Scene::updateInstancedMeshTransform(IPLInstancedMesh mesh, const IPLMatrix4x4& transform) {
	if (mesh && scene) iplInstancedMeshUpdateTransform(mesh, scene, transform);
}

} // namespace ofxSteamAudio
