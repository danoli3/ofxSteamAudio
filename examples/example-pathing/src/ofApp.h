#pragma once
#include "ofMain.h"
#include "ofxSteamAudio.h"
#include <vector>

/// de_dust2–inspired A-site + Long layout with controllable listener & emitter.
class ofApp : public ofBaseApp {
public:
	void setup() override;
	void update() override;
	void draw() override;
	void audioOut(ofSoundBuffer& buffer) override;
	void keyPressed(int key) override;
	void keyReleased(int key) override;
	void mousePressed(int x, int y, int button) override;
	void mouseDragged(int x, int y, int button) override;
	void mouseReleased(int x, int y, int button) override;

private:
	enum class MatKind { Plaster, Brick, Wood, Concrete, Metal, Sand };

	struct Wall {
		glm::vec3 center;
		glm::vec3 size; // width (X), height (Y), depth (Z)
		ofColor color;
		MatKind mat = MatKind::Plaster;
		ofBoxPrimitive mesh;
	};

	void resetCamera();
	void resetActors();
	void buildDust2ASite();
	void addWall(const glm::vec3& center, const glm::vec3& size, MatKind mat, ofColor color);
	void rebuildProbes();
	void bindSimulator();
	void runDirectSim();
	void drawHelp() const;
	void drawLabels() const;
	void moveSelected(const glm::vec3& delta);
	glm::vec3 clampToMap(const glm::vec3& p) const;
	bool rayPlaneY(const glm::vec2& screen, float y, glm::vec3& hit) const;
	static IPLMaterial iplMat(MatKind k);

	ofEasyCam cam;

	// Steam Audio
	ofxSteamAudio::Context context;
	ofxSteamAudio::HRTF hrtf;
	ofxSteamAudio::BinauralEffect binaural;
	ofxSteamAudio::DirectEffect direct;
	ofxSteamAudio::PathEffect pathEffect;
	ofxSteamAudio::Scene scene;
	ofxSteamAudio::Simulator simulator;
	ofxSteamAudio::SimSource simSource;
	ofxSteamAudio::ProbeArray probes;
	ofxSteamAudio::ProbeBatch probeBatch;
	ofxSteamAudio::AudioBuffer monoIn, directOut, stereoOut;
	IPLAudioSettings audioSettings{};
	ofSoundStream soundStream;

	// Visual geometry
	std::vector<Wall> walls;
	ofPlanePrimitive ground;
	std::vector<IPLSphere> probeSpheres;
	bool showProbes = true;
	bool showLabels = true;

	// Controllable poses (world meters, Y-up)
	// Long runs along +Z toward A site; site sits at high Z.
	glm::vec3 listenerPos{ 1.5f, 1.6f, 12.0f };   // on A site (CT)
	glm::vec3 sourcePos  { 0.0f, 1.6f, -22.0f };  // T side of Long
	glm::vec3 listenerAhead{ 0.0f, 0.0f, -1.0f };
	enum class Selection { Listener, Source };
	Selection selection = Selection::Source;

	// Map bounds (for clamp + ground)
	float mapMinX = -14.0f, mapMaxX = 16.0f;
	float mapMinZ = -30.0f, mapMaxZ = 22.0f;
	float wallHeight = 4.0f;

	// Interaction
	bool keys[512]{};
	bool dragging = false;
	float moveSpeed = 6.0f; // m/s — larger map

	// Audio state
	float phase = 0.0f;
	float lastOcclusion = 0.0f;
	float lastAttenuation = 1.0f;
	bool ok = false;
};
