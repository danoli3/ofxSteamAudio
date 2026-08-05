#include "ofApp.h"

// -----------------------------------------------------------------------------
void ofApp::setup() {
	ofSetWindowTitle("ofxSteamAudio · Pathing room (controllable listener + emitter)");
	ofSetFrameRate(60);
	ofEnableDepthTest();
	ofEnableAlphaBlending();

	cam.setDistance(18);
	cam.setNearClip(0.1f);
	cam.setFarClip(200);
	cam.setTarget(glm::vec3(0, 1.0f, 0));

	audioSettings.samplingRate = 44100;
	audioSettings.frameSize = 512;

	ok = context.setup()
		&& hrtf.create(context, audioSettings)
		&& binaural.create(context, audioSettings, hrtf)
		&& direct.create(context, audioSettings, 1)
		&& pathEffect.create(context, audioSettings, 1, true, hrtf)
		&& monoIn.allocate(context, 1, audioSettings.frameSize)
		&& directOut.allocate(context, 1, audioSettings.frameSize)
		&& stereoOut.allocate(context, 2, audioSettings.frameSize)
		&& scene.create(context)
		&& simulator.create(context, audioSettings, IPL_SIMULATIONFLAGS_DIRECT)
		&& probes.create(context)
		&& probeBatch.create(context);

	if (!ok) {
		ofLogError() << "ofxSteamAudio setup failed";
		return;
	}

	buildRoom();
	rebuildProbes();

	// Direct sim source (occlusion + distance through walls)
	simulator.setScene(scene);
	simulator.commit();
	simSource.create(simulator.get(), IPL_SIMULATIONFLAGS_DIRECT);
	simSource.add(simulator.get());
	simulator.commit();

	ofSoundStreamSettings s;
	s.numOutputChannels = 2;
	s.numInputChannels = 0;
	s.sampleRate = audioSettings.samplingRate;
	s.bufferSize = audioSettings.frameSize;
	s.numBuffers = 4;
	s.setOutListener(this);
	soundStream.setup(s);

	ofLogNotice() << "Controls: Tab select · WASD move · Q/E height · drag on ground · P probes · R reset";
}

// -----------------------------------------------------------------------------
void ofApp::buildRoom() {
	walls.clear();
	scene.release();
	scene.create(context);

	const float h = wallHeight;
	const float t = 0.25f; // wall thickness
	const float half = roomHalf;

	// Solid ground (acoustic) — large floor plane on Y=0
	scene.addGroundPlane(half + 2.0f, ofxSteamAudio::Materials::concrete());

	// Outer walls (4 sides) + center divider with doorway gap (two segments)
	struct Spec { glm::vec3 c, s; ofColor col; };
	const ofColor plasterCol(190, 185, 175, 220);
	const ofColor woodCol(140, 100, 60, 230);
	const ofColor brickCol(160, 90, 70, 220);

	std::vector<Spec> specs = {
		// Outer shell
		{ { 0, h * 0.5f, -half }, { half * 2.0f, h, t }, plasterCol }, // north (-Z)
		{ { 0, h * 0.5f,  half }, { half * 2.0f, h, t }, plasterCol }, // south (+Z)
		{ { -half, h * 0.5f, 0 }, { t, h, half * 2.0f }, plasterCol }, // west
		{ {  half, h * 0.5f, 0 }, { t, h, half * 2.0f }, plasterCol }, // east
		// Center divider on XZ with gap in the middle (pathing around doorway)
		// left segment and right segment leave ~2m opening at x=0
		{ { -2.5f, h * 0.5f, 0 }, { 3.0f, h, t }, woodCol },
		{ {  2.5f, h * 0.5f, 0 }, { 3.0f, h, t }, woodCol },
		// Small side barrier for more interesting occlusion
		{ { 2.0f, h * 0.5f, -2.5f }, { t, h, 2.5f }, brickCol },
	};

	for (const auto& sp : specs) {
		Wall w;
		w.center = sp.c;
		w.size = sp.s;
		w.color = sp.col;
		w.mesh.set(sp.s.x, sp.s.y, sp.s.z);
		w.mesh.setPosition(sp.c);
		walls.push_back(w);

		const bool wood = (sp.col == woodCol);
		const bool brick = (sp.col == brickCol);
		IPLMaterial mat = wood ? ofxSteamAudio::Materials::wood()
			: brick ? ofxSteamAudio::Materials::brick()
			: ofxSteamAudio::Materials::plaster();
		scene.addBox(sp.s.x, sp.s.y, sp.s.z, mat, sp.c);
	}
	scene.commit();

	// Visual ground plane (solid under everything)
	const float groundSize = (half + 2.0f) * 2.0f;
	ground.set(groundSize, groundSize);
	ground.setResolution(20, 20);
	ground.setPosition(0, 0, 0);
	// ofPlanePrimitive is XY by default; rotate to XZ (lie flat)
	ground.rotateDeg(-90, 1, 0, 0);
}

// -----------------------------------------------------------------------------
void ofApp::rebuildProbes() {
	probes.release();
	probeBatch.release();
	probes.create(context);
	probeBatch.create(context);
	probeSpheres.clear();

	IPLProbeGenerationParams pg{};
	pg.type = IPL_PROBEGENERATIONTYPE_UNIFORMFLOOR;
	pg.spacing = 1.5f;
	pg.height = 1.5f;
	pg.transform = ofxSteamAudio::identityMatrix();
	// Unit cube → world AABB [-roomHalf..roomHalf] roughly
	const float extent = roomHalf * 2.0f;
	pg.transform.elements[0][0] = extent;
	pg.transform.elements[1][1] = wallHeight;
	pg.transform.elements[2][2] = extent;
	pg.transform.elements[0][3] = -roomHalf;
	pg.transform.elements[1][3] = 0;
	pg.transform.elements[2][3] = -roomHalf;

	probes.generate(scene, pg);
	probeBatch.addProbeArray(probes.get());
	probeBatch.commit();

	const int n = probes.numProbes();
	for (int i = 0; i < n; ++i) probeSpheres.push_back(probes.getProbe(i));
	ofLogNotice() << "Probes: " << n;
}

// -----------------------------------------------------------------------------
void ofApp::runDirectSim() {
	if (!ok) return;

	IPLSimulationSharedInputs shared{};
	shared.listener = ofxSteamAudio::makeCoordinateSpace(listenerPos, listenerAhead, glm::vec3(0, 1, 0));
	shared.numRays = 0;
	shared.numBounces = 0;
	shared.duration = 1.0f;
	shared.order = 0;
	shared.irradianceMinDistance = 1.0f;
	simulator.setSharedInputs(IPL_SIMULATIONFLAGS_DIRECT, &shared);

	IPLSimulationInputs inputs{};
	inputs.flags = IPL_SIMULATIONFLAGS_DIRECT;
	inputs.directFlags = static_cast<IPLDirectSimulationFlags>(
		IPL_DIRECTSIMULATIONFLAGS_DISTANCEATTENUATION |
		IPL_DIRECTSIMULATIONFLAGS_AIRABSORPTION |
		IPL_DIRECTSIMULATIONFLAGS_OCCLUSION |
		IPL_DIRECTSIMULATIONFLAGS_TRANSMISSION);
	inputs.source = ofxSteamAudio::makeCoordinateSpace(sourcePos);
	inputs.distanceAttenuationModel.type = IPL_DISTANCEATTENUATIONTYPE_DEFAULT;
	inputs.airAbsorptionModel.type = IPL_AIRABSORPTIONTYPE_DEFAULT;
	inputs.occlusionType = IPL_OCCLUSIONTYPE_RAYCAST;
	inputs.occlusionRadius = 0.15f;
	inputs.numOcclusionSamples = 4;
	simSource.setInputs(IPL_SIMULATIONFLAGS_DIRECT, &inputs);

	simulator.runDirect();

	IPLSimulationOutputs outputs{};
	simSource.getOutputs(IPL_SIMULATIONFLAGS_DIRECT, &outputs);
	lastOcclusion = outputs.direct.occlusion;
	lastAttenuation = outputs.direct.distanceAttenuation;
}

// -----------------------------------------------------------------------------
glm::vec3 ofApp::clampToRoom(const glm::vec3& p) const {
	const float m = roomHalf - 0.4f;
	return {
		ofClamp(p.x, -m, m),
		ofClamp(p.y, 0.3f, wallHeight - 0.2f),
		ofClamp(p.z, -m, m)
	};
}

void ofApp::moveSelected(const glm::vec3& delta) {
	if (selection == Selection::Listener) {
		listenerPos = clampToRoom(listenerPos + delta);
	} else {
		sourcePos = clampToRoom(sourcePos + delta);
	}
}

// -----------------------------------------------------------------------------
void ofApp::update() {
	if (!ok) return;

	const float dt = ofGetLastFrameTime();
	const float step = moveSpeed * dt;

	// Camera-relative horizontal axes for WASD
	glm::vec3 ahead = cam.getLookAtDir();
	ahead.y = 0;
	if (glm::length2(ahead) > 1e-6f) ahead = glm::normalize(ahead);
	else ahead = glm::vec3(0, 0, -1);
	glm::vec3 right = glm::normalize(glm::cross(ahead, glm::vec3(0, 1, 0)));

	glm::vec3 delta(0);
	if (keys['w'] || keys['W'] || keys[OF_KEY_UP])    delta += ahead * step;
	if (keys['s'] || keys['S'] || keys[OF_KEY_DOWN])  delta -= ahead * step;
	if (keys['a'] || keys['A'] || keys[OF_KEY_LEFT])  delta -= right * step;
	if (keys['d'] || keys['D'] || keys[OF_KEY_RIGHT]) delta += right * step;
	if (keys['q'] || keys['Q']) delta.y -= step;
	if (keys['e'] || keys['E']) delta.y += step;

	if (glm::length2(delta) > 0) moveSelected(delta);

	// Listener faces the source (for orientation / spatialization)
	glm::vec3 toSrc = sourcePos - listenerPos;
	toSrc.y = 0;
	if (glm::length2(toSrc) > 1e-4f) listenerAhead = glm::normalize(toSrc);

	runDirectSim();
}

// -----------------------------------------------------------------------------
void ofApp::draw() {
	ofBackground(28, 30, 36);

	cam.begin();

	// Solid ground (XZ plane)
	ofSetColor(55, 58, 62);
	ground.draw();
	// Grid on ground for scale
	ofSetColor(70, 74, 80);
	ofPushMatrix();
	ofRotateXDeg(90);
	ofDrawGridPlane(roomHalf * 2.0f, 20, false);
	ofPopMatrix();

	// 3D walls
	ofEnableBlendMode(OF_BLENDMODE_ALPHA);
	for (auto& w : walls) {
		ofSetColor(w.color);
		w.mesh.draw();
		ofSetColor(30, 30, 30, 180);
		w.mesh.drawWireframe();
	}
	ofEnableBlendMode(OF_BLENDMODE_ALPHA);

	// Probes (pathing sample points)
	if (showProbes) {
		ofSetColor(80, 170, 255, 120);
		for (const auto& p : probeSpheres) {
			ofDrawSphere(p.center.x, p.center.y, p.center.z, 0.10f);
		}
	}

	// Line of sight listener → source (green if clear-ish, red if occluded)
	{
		const float occ = lastOcclusion;
		ofSetColor(ofColor::fromHsb(ofMap(occ, 0, 1, 90, 0, true), 200, 255));
		ofDrawLine(listenerPos, sourcePos);
	}

	// Listener sphere (cyan) — controllable
	{
		const bool sel = (selection == Selection::Listener);
		ofSetColor(sel ? ofColor(80, 230, 255) : ofColor(40, 160, 200));
		ofDrawSphere(listenerPos, sel ? 0.28f : 0.22f);
		// Orientation arrow
		ofSetColor(255, 255, 100);
		ofDrawArrow(listenerPos, listenerPos + listenerAhead * 0.7f, 0.08f);
		ofSetColor(255);
		ofDrawBitmapString("LISTENER", listenerPos + glm::vec3(0, 0.45f, 0));
	}

	// Source / emitter sphere (orange-red) — controllable
	{
		const bool sel = (selection == Selection::Source);
		ofSetColor(sel ? ofColor(255, 120, 60) : ofColor(200, 70, 40));
		ofDrawSphere(sourcePos, sel ? 0.28f : 0.22f);
		// Pulse ring
		ofNoFill();
		ofSetCircleResolution(32);
		ofPushMatrix();
		ofTranslate(sourcePos);
		ofRotateXDeg(90);
		float r = 0.35f + 0.08f * sinf(ofGetElapsedTimef() * 6.0f);
		ofDrawCircle(0, 0, r);
		ofPopMatrix();
		ofFill();
		ofSetColor(255);
		ofDrawBitmapString("EMITTER", sourcePos + glm::vec3(0, 0.45f, 0));
	}

	cam.end();

	drawHelp();
}

// -----------------------------------------------------------------------------
void ofApp::drawHelp() const {
	const std::string selName = (selection == Selection::Listener) ? "LISTENER" : "EMITTER";
	ofDrawBitmapStringHighlight(
		"3D walls + solid ground · pathing probes\n"
		"Selected: " + selName + "  [Tab] switch\n"
		"Move: WASD / arrows  height: Q/E\n"
		"Drag LMB on ground to place selected (cam: RMB/scroll)\n"
		"[P] probes  [R] reset positions\n"
		"attenuation=" + ofToString(lastAttenuation, 2) +
		"  occlusion=" + ofToString(lastOcclusion, 2) +
		"  probes=" + ofToString((int)probeSpheres.size()),
		14, 22);
}

// -----------------------------------------------------------------------------
void ofApp::audioOut(ofSoundBuffer& buffer) {
	if (!ok) {
		buffer.set(0);
		return;
	}

	monoIn.fillSine(240.0f, (float)audioSettings.samplingRate, 0.45f, phase);

	// Latest direct-path params from simulation thread-safe-ish snapshot
	IPLSimulationOutputs outputs{};
	simSource.getOutputs(IPL_SIMULATIONFLAGS_DIRECT, &outputs);

	IPLDirectEffectParams dp = outputs.direct;
	dp.flags = static_cast<IPLDirectEffectFlags>(
		IPL_DIRECTEFFECTFLAGS_APPLYDISTANCEATTENUATION |
		IPL_DIRECTEFFECTFLAGS_APPLYAIRABSORPTION |
		IPL_DIRECTEFFECTFLAGS_APPLYOCCLUSION |
		IPL_DIRECTEFFECTFLAGS_APPLYTRANSMISSION);
	dp.transmissionType = IPL_TRANSMISSIONTYPE_FREQINDEPENDENT;
	// Mild transmission so fully occluded still faintly audible through walls
	if (dp.transmission[0] <= 0.0f && dp.occlusion > 0.5f) {
		dp.transmission[0] = dp.transmission[1] = dp.transmission[2] = 0.15f;
	}

	direct.apply(dp, monoIn.get(), directOut.get());

	IPLVector3 dir = ofxSteamAudio::relativeDirection(
		context, sourcePos, listenerPos, listenerAhead, glm::vec3(0, 1, 0));
	binaural.apply(dir, hrtf, directOut.get(), stereoOut.get());

	buffer.set(0);
	stereoOut.mixTo(buffer);
}

// -----------------------------------------------------------------------------
void ofApp::keyPressed(int key) {
	if (key >= 0 && key < 512) keys[key] = true;

	if (key == OF_KEY_TAB) {
		selection = (selection == Selection::Listener) ? Selection::Source : Selection::Listener;
	}
	if (key == 'p' || key == 'P') showProbes = !showProbes;
	if (key == 'r' || key == 'R') {
		listenerPos = { 0.0f, 1.6f,  3.5f };
		sourcePos   = { 0.0f, 1.6f, -3.5f };
	}
}

void ofApp::keyReleased(int key) {
	if (key >= 0 && key < 512) keys[key] = false;
}

// Ray vs Y plane for drag-place
bool ofApp::rayPlaneY(const glm::vec2& screen, float y, glm::vec3& hit) const {
	// ofEasyCam: unproject near/far
	glm::vec3 nearW = cam.screenToWorld(glm::vec3(screen.x, screen.y, 0));
	glm::vec3 farW  = cam.screenToWorld(glm::vec3(screen.x, screen.y, 1));
	glm::vec3 dir = farW - nearW;
	if (std::fabs(dir.y) < 1e-6f) return false;
	float t = (y - nearW.y) / dir.y;
	if (t < 0) return false;
	hit = nearW + dir * t;
	return true;
}

void ofApp::mousePressed(int x, int y, int button) {
	if (button != OF_MOUSE_BUTTON_LEFT) return;
	// Only drag when shift held so easyCam can still orbit with left otherwise —
	// actually ofEasyCam uses left drag for orbit. Use SHIFT+LMB for place.
	if (!ofGetKeyPressed(OF_KEY_SHIFT)) return;
	dragging = true;
	cam.disableMouseInput();

	glm::vec3 hit;
	const float placeY = (selection == Selection::Listener) ? listenerPos.y : sourcePos.y;
	if (rayPlaneY(glm::vec2(x, y), placeY, hit)) {
		hit.y = placeY;
		if (selection == Selection::Listener) listenerPos = clampToRoom(hit);
		else sourcePos = clampToRoom(hit);
	}
}

void ofApp::mouseDragged(int x, int y, int button) {
	if (!dragging || button != OF_MOUSE_BUTTON_LEFT) return;
	glm::vec3 hit;
	const float placeY = (selection == Selection::Listener) ? listenerPos.y : sourcePos.y;
	if (rayPlaneY(glm::vec2(x, y), placeY, hit)) {
		hit.y = placeY;
		if (selection == Selection::Listener) listenerPos = clampToRoom(hit);
		else sourcePos = clampToRoom(hit);
	}
}

void ofApp::mouseReleased(int x, int y, int button) {
	if (button == OF_MOUSE_BUTTON_LEFT && dragging) {
		dragging = false;
		cam.enableMouseInput();
	}
}
