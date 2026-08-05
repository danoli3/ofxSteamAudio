#pragma once
#include "ofMain.h"
#include "ofxSteamAudio.h"

/// Scene / static mesh geometry demo — maps to itest: scene, staticmesh, instancedmesh
class ofApp : public ofBaseApp {
public:
	void setup() override;
	void update() override;
	void draw() override;
	void keyPressed(int key) override;

	ofEasyCam cam;
	ofxSteamAudio::Context context;
	ofxSteamAudio::Scene scene;
	ofxSteamAudio::Scene doorSubscene;
	IPLInstancedMesh doorMesh = nullptr;
	ofBoxPrimitive wallBox, floorBox, doorBox;
	float doorAngle = 0;
	bool ok = false;
};
