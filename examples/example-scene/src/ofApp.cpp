#include "ofApp.h"

void ofApp::setup() {
	ofSetWindowTitle("ofxSteamAudio · Scene / StaticMesh");
	ofSetFrameRate(60);
	ofEnableDepthTest();
	cam.setDistance(15);

	ok = context.setup() && scene.create(context) && doorSubscene.create(context);
	if (!ok) return;

	// Room shell pieces (visual + acoustic)
	scene.addGroundPlane(20.f, ofxSteamAudio::Materials::concrete());
	scene.addBox(8, 3, 0.3f, ofxSteamAudio::Materials::brick(), glm::vec3(0, 1.5f, -4));
	scene.addBox(0.3f, 3, 8, ofxSteamAudio::Materials::brick(), glm::vec3(-4, 1.5f, 0));
	scene.addBox(0.3f, 3, 8, ofxSteamAudio::Materials::brick(), glm::vec3(4, 1.5f, 0));

	// Door as sub-scene (instanced, can rotate)
	doorSubscene.addBox(1.0f, 2.2f, 0.1f, ofxSteamAudio::Materials::wood(), glm::vec3(0, 1.1f, 0));
	doorSubscene.commit();

	IPLMatrix4x4 xform = ofxSteamAudio::identityMatrix();
	xform.elements[0][3] = 0; // translation in last column? row-major affine
	// Steam Audio matrix is row-major; translation typically in elements[i][3]
	xform.elements[0][3] = 0.0f;
	xform.elements[1][3] = 0.0f;
	xform.elements[2][3] = 3.5f;
	doorMesh = scene.addInstancedMesh(doorSubscene.get(), xform);
	scene.commit();

	wallBox.set(8, 3, 0.3f);
	wallBox.setPosition(0, 1.5f, -4);
	floorBox.set(20, 0.05f, 20);
	floorBox.setPosition(0, 0, 0);
	doorBox.set(1.0f, 2.2f, 0.1f);
}

void ofApp::update() {
	if (!ok || !doorMesh) return;
	doorAngle = ofGetElapsedTimef() * 0.4f;
	// Simple Y rotation + translate
	IPLMatrix4x4 xform = ofxSteamAudio::identityMatrix();
	float c = cosf(doorAngle), s = sinf(doorAngle);
	xform.elements[0][0] = c;  xform.elements[0][2] = s;
	xform.elements[2][0] = -s; xform.elements[2][2] = c;
	xform.elements[0][3] = 0;
	xform.elements[1][3] = 0;
	xform.elements[2][3] = 3.5f;
	scene.updateInstancedMeshTransform(doorMesh, xform);
	scene.commit();

	doorBox.setPosition(0, 1.1f, 3.5f);
	doorBox.setOrientation(glm::angleAxis(doorAngle, glm::vec3(0, 1, 0)));
}

void ofApp::draw() {
	ofBackground(30);
	cam.begin();
	ofSetColor(120);
	floorBox.draw();
	ofSetColor(160, 100, 80);
	wallBox.draw();
	ofSetColor(180, 140, 90);
	doorBox.draw();
	ofSetColor(255, 80, 80);
	ofDrawSphere(0, 1.6f, 0, 0.15f); // listener
	cam.end();

	ofDrawBitmapStringHighlight(
		"Scene + StaticMesh + InstancedMesh\n"
		"Door rotates as instanced mesh (for occlusion/pathing)\n"
		"Press S to export OBJ of scene", 20, 30);
}

void ofApp::keyPressed(int key) {
	if (!ok) return;
	if (key == 's' || key == 'S') {
		std::string path = ofToDataPath("scene_export", true);
		scene.saveOBJ(path);
		ofLogNotice() << "Saved OBJ base: " << path;
	}
}
