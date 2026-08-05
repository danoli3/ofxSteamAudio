#pragma once
#include "ofMain.h"
#include "ofxSteamAudio.h"
#include <vector>

/// Pathing / probes demo with visible 3D walls, solid ground,
/// and keyboard-controllable listener + emitter spheres.
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
	struct Wall {
		glm::vec3 center;
		glm::vec3 size; // width, height, depth
		ofColor color;
		ofBoxPrimitive mesh;
	};

	void buildRoom();
	void rebuildProbes();
	void runDirectSim();
	void drawHelp() const;
	void moveSelected(const glm::vec3& delta);
	glm::vec3 clampToRoom(const glm::vec3& p) const;
	bool rayPlaneY(const glm::vec2& screen, float y, glm::vec3& hit) const;

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

	// Controllable poses (world meters, Y-up)
	glm::vec3 listenerPos{ 0.0f, 1.6f,  3.5f };
	glm::vec3 sourcePos  { 0.0f, 1.6f, -3.5f };
	glm::vec3 listenerAhead{ 0.0f, 0.0f, -1.0f }; // face -Z by default
	enum class Selection { Listener, Source };
	Selection selection = Selection::Source;

	// Interaction
	bool keys[512]{};
	bool dragging = false;
	float moveSpeed = 4.0f; // m/s
	float roomHalf = 5.0f;
	float wallHeight = 3.0f;

	// Audio state
	float phase = 0.0f;
	float lastOcclusion = 0.0f;
	float lastAttenuation = 1.0f;
	bool ok = false;
};
