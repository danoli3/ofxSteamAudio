#include "ofApp.h"

// =============================================================================
// de_dust2 A-site + Long (simplified architectural massing for occlusion)
//
// Coordinate system (meters):
//   +Y up
//   +Z  toward A site (from T Long)
//   +X  toward Short / Cat / CT side of site
//
// Layout (top-down):
//
//   z≈20   ████ back of A / CT ramp wall ████
//   z≈14   ║  goose   ║   plant / default  ║  platform boxes
//   z≈8    ║──────────╫─ ramp / site front ╫── short mouth ──
//   z≈2    ║  car     ║
//   z≈0    ║  LONG A corridor (long walls)  ║── short alley ──
//   z≈-24  ║  T long start                  ║
// =============================================================================

namespace {
	const ofColor kSand(194, 178, 128, 255);
	const ofColor kPlaster(210, 200, 185, 230);
	const ofColor kBrick(150, 85, 65, 235);
	const ofColor kWood(120, 85, 50, 235);
	const ofColor kConcrete(120, 120, 118, 240);
	const ofColor kMetal(90, 95, 100, 240);
	const ofColor kDark(70, 65, 55, 245);
}

IPLMaterial ofApp::iplMat(MatKind k) {
	namespace M = ofxSteamAudio::Materials;
	switch (k) {
		case MatKind::Brick:    return M::brick();
		case MatKind::Wood:     return M::wood();
		case MatKind::Concrete: return M::concrete();
		case MatKind::Metal:    return M::metal();
		case MatKind::Sand:     return M::rock();
		default:                return M::plaster();
	}
}

// -----------------------------------------------------------------------------
void ofApp::setup() {
	ofSetWindowTitle("ofxSteamAudio · de_dust2 A-site + Long");
	ofSetFrameRate(60);
	ofEnableDepthTest();
	ofEnableAlphaBlending();
	ofSetCircleResolution(40);

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

	buildDust2ASite();
	rebuildProbes();
	bindSimulator();
	resetActors();
	resetCamera();

	ofSoundStreamSettings s;
	s.numOutputChannels = 2;
	s.numInputChannels = 0;
	s.sampleRate = audioSettings.samplingRate;
	s.bufferSize = audioSettings.frameSize;
	s.numBuffers = 4;
	s.setOutListener(this);
	soundStream.setup(s);

	ofLogNotice() << "de_dust2 A + Long: Tab select · WASD move · Q/E height · Shift+drag place · R reset all · P probes · L labels";
}

// -----------------------------------------------------------------------------
void ofApp::resetCamera() {
	// Elevated view looking down Long toward A site
	cam.setNearClip(0.2f);
	cam.setFarClip(400);
	cam.setDistance(55);
	cam.setTarget(glm::vec3(2.0f, 0.5f, -2.0f));
	cam.setPosition(28.0f, 32.0f, -8.0f);
	cam.lookAt(glm::vec3(2.0f, 1.0f, 2.0f));
}

void ofApp::resetActors() {
	// CT holding A default / plant side
	listenerPos = { 1.5f, 1.6f, 12.0f };
	// T peeking Long
	sourcePos = { 0.0f, 1.6f, -22.0f };
	listenerAhead = glm::normalize(sourcePos - listenerPos);
	listenerAhead.y = 0;
	if (glm::length2(listenerAhead) < 1e-4f) listenerAhead = { 0, 0, -1 };
	else listenerAhead = glm::normalize(listenerAhead);
	selection = Selection::Source;
}

// -----------------------------------------------------------------------------
void ofApp::addWall(const glm::vec3& center, const glm::vec3& size, MatKind mat, ofColor color) {
	Wall w;
	w.center = center;
	w.size = size;
	w.color = color;
	w.mat = mat;
	w.mesh.set(size.x, size.y, size.z);
	w.mesh.setPosition(center);
	walls.push_back(w);
	scene.addBox(size.x, size.y, size.z, iplMat(mat), center);
}

// -----------------------------------------------------------------------------
void ofApp::buildDust2ASite() {
	walls.clear();
	scene.release();
	scene.create(context);

	const float h = wallHeight;
	const float t = 0.35f;       // wall thickness
	const float hh = h * 0.5f;   // wall center Y

	// Large ground under whole map
	const float groundHalf = 40.0f;
	scene.addGroundPlane(groundHalf, ofxSteamAudio::Materials::concrete());

	// ---------- LONG A corridor (runs along Z, roughly x ∈ [-4, 4]) ----------
	// Outer long walls
	addWall({ -4.0f, hh, -8.0f }, { t, h, 36.0f }, MatKind::Sand, kSand);     // long left
	addWall({  4.0f, hh, -12.0f }, { t, h, 28.0f }, MatKind::Sand, kSand);    // long right (shorter: opens near site)

	// Long back / T-side end wall with opening? keep closed-ish for acoustic box
	addWall({ 0.0f, hh, -27.0f }, { 8.5f, h, t }, MatKind::Brick, kBrick);

	// Pillars / niches along long (break LOS a bit)
	addWall({ -2.8f, hh * 0.7f, -18.0f }, { 1.2f, h * 0.7f, 1.2f }, MatKind::Concrete, kConcrete);
	addWall({  2.6f, hh * 0.7f, -10.0f }, { 1.4f, h * 0.7f, 1.4f }, MatKind::Concrete, kConcrete);
	addWall({ -2.5f, hh * 0.6f,  -4.0f }, { 1.5f, h * 0.6f, 1.8f }, MatKind::Brick, kBrick);

	// "Car" at long-to-site corner (classic dust2 blue car massing)
	addWall({ 2.2f, 0.7f, 1.5f }, { 2.2f, 1.4f, 4.5f }, MatKind::Metal, kMetal);

	// Long-to-site corner walls (blue container / corner)
	addWall({ 5.5f, hh, 2.0f }, { t, h, 8.0f }, MatKind::Metal, ofColor(50, 80, 140, 240));
	addWall({ 3.0f, hh, 5.5f }, { 5.0f, h, t }, MatKind::Metal, ofColor(50, 80, 140, 240));

	// ---------- SHORT A / CAT mouth (positive X, mid Z) ----------
	// Short corridor walls toward mid
	addWall({ 9.0f, hh, -2.0f }, { t, h, 14.0f }, MatKind::Sand, kSand);      // short outer
	addWall({ 6.0f, hh, -6.0f }, { t, h, 6.0f }, MatKind::Plaster, kPlaster); // short inner stub
	// Short floor-level stairs mass (raised box)
	addWall({ 7.5f, 0.45f, 3.0f }, { 3.5f, 0.9f, 4.0f }, MatKind::Concrete, kConcrete);

	// ---------- A SITE (z ≈ 6 … 20, x ≈ -6 … 12) ----------
	// Site left wall (toward pit / goose)
	addWall({ -6.0f, hh, 12.0f }, { t, h, 16.0f }, MatKind::Sand, kSand);
	// Site back wall (CT)
	addWall({ 2.0f, hh, 20.0f }, { 18.0f, h, t }, MatKind::Brick, kBrick);
	// Site right wall (toward CT spawn / platform)
	addWall({ 12.0f, hh, 12.0f }, { t, h, 16.0f }, MatKind::Sand, kSand);

	// Goose / default boxes (left of plant)
	addWall({ -3.5f, 0.9f, 14.0f }, { 2.5f, 1.8f, 2.5f }, MatKind::Wood, kWood);
	addWall({ -3.2f, 1.5f, 14.2f }, { 1.4f, 1.0f, 1.4f }, MatKind::Wood, kWood); // stacked

	// Default / plant boxes (center-left)
	addWall({ 0.5f, 0.7f, 13.5f }, { 2.0f, 1.4f, 2.0f }, MatKind::Wood, kWood);
	addWall({ 2.2f, 0.55f, 15.0f }, { 1.6f, 1.1f, 1.6f }, MatKind::Wood, kWood);

	// Ninja / dark corner stack near back-left
	addWall({ -4.5f, 1.0f, 17.5f }, { 1.8f, 2.0f, 1.8f }, MatKind::Wood, kDark);

	// Platform / scaffold on CT side of site (raised)
	addWall({ 8.5f, 1.2f, 14.5f }, { 5.0f, 2.4f, 4.0f }, MatKind::Concrete, kConcrete);
	addWall({ 9.5f, 2.6f, 14.5f }, { 2.5f, 0.4f, 2.5f }, MatKind::Wood, kWood); // top crate

	// Site front wall segments (partial LOS blockers toward long) with openings
	// Leaves gaps: long entry (~x -1..3) and short entry (~x 6..10)
	addWall({ -3.5f, hh, 7.5f }, { 5.0f, h, t }, MatKind::Sand, kSand);  // left of long mouth
	addWall({  5.0f, hh, 7.5f }, { 3.0f, h, t }, MatKind::Sand, kSand);  // between long & short mouths
	addWall({ 11.0f, hh, 7.5f }, { 2.5f, h, t }, MatKind::Sand, kSand); // right of short mouth

	// Ramp / slope mass into site from long (angled feel via stepped boxes)
	addWall({ 0.0f, 0.35f, 6.0f }, { 5.0f, 0.7f, 2.5f }, MatKind::Concrete, kConcrete);
	addWall({ 0.0f, 0.70f, 7.8f }, { 4.5f, 0.7f, 2.0f }, MatKind::Concrete, kConcrete);

	// Barrel stacks (small blockers)
	addWall({ 4.0f, 0.6f, 11.0f }, { 0.9f, 1.2f, 0.9f }, MatKind::Metal, kMetal);
	addWall({ 4.9f, 0.6f, 11.3f }, { 0.9f, 1.2f, 0.9f }, MatKind::Metal, kMetal);
	addWall({ -1.0f, 0.55f, 10.0f }, { 0.8f, 1.1f, 0.8f }, MatKind::Metal, kMetal);

	// Mid-ish connector wall near short (helps block diagonal LOS)
	addWall({ 6.5f, hh, 0.5f }, { 4.0f, h, t }, MatKind::Brick, kBrick);

	// Elevator-style pillar near car
	addWall({ -1.5f, hh, 2.5f }, { 1.5f, h, 1.5f }, MatKind::Concrete, kConcrete);

	scene.commit();

	// Visual ground
	const float gw = std::max(mapMaxX - mapMinX, mapMaxZ - mapMinZ) + 20.0f;
	ground.set(gw, gw);
	ground.setResolution(32, 32);
	ground.setPosition((mapMinX + mapMaxX) * 0.5f, 0, (mapMinZ + mapMaxZ) * 0.5f);
	ground.setOrientation(glm::angleAxis(glm::radians(-90.0f), glm::vec3(1, 0, 0)));

	ofLogNotice() << "Built de_dust2 A-site + Long massing: " << walls.size() << " solids";
}

// -----------------------------------------------------------------------------
void ofApp::rebuildProbes() {
	probes.release();
	probeBatch.release();
	probes.create(context);
	probeBatch.create(context);
	probeSpheres.clear();

	// Probe volume covering long + site
	IPLProbeGenerationParams pg{};
	pg.type = IPL_PROBEGENERATIONTYPE_UNIFORMFLOOR;
	pg.spacing = 3.0f; // larger map → coarser grid
	pg.height = 1.5f;
	pg.transform = ofxSteamAudio::identityMatrix();
	const float sx = mapMaxX - mapMinX;
	const float sz = mapMaxZ - mapMinZ;
	pg.transform.elements[0][0] = sx;
	pg.transform.elements[1][1] = wallHeight;
	pg.transform.elements[2][2] = sz;
	pg.transform.elements[0][3] = mapMinX;
	pg.transform.elements[1][3] = 0;
	pg.transform.elements[2][3] = mapMinZ;

	probes.generate(scene, pg);
	probeBatch.addProbeArray(probes.get());
	probeBatch.commit();

	const int n = probes.numProbes();
	for (int i = 0; i < n; ++i) probeSpheres.push_back(probes.getProbe(i));
	ofLogNotice() << "Probes: " << n;
}

void ofApp::bindSimulator() {
	// Recreate sim source against current scene
	simSource.release();
	simulator.setScene(scene);
	simulator.commit();
	simSource.create(simulator.get(), IPL_SIMULATIONFLAGS_DIRECT);
	simSource.add(simulator.get());
	simulator.commit();
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
	inputs.occlusionRadius = 0.2f;
	inputs.numOcclusionSamples = 8;
	simSource.setInputs(IPL_SIMULATIONFLAGS_DIRECT, &inputs);

	simulator.runDirect();

	IPLSimulationOutputs outputs{};
	simSource.getOutputs(IPL_SIMULATIONFLAGS_DIRECT, &outputs);
	lastOcclusion = outputs.direct.occlusion;
	lastAttenuation = outputs.direct.distanceAttenuation;
}

// -----------------------------------------------------------------------------
glm::vec3 ofApp::clampToMap(const glm::vec3& p) const {
	return {
		ofClamp(p.x, mapMinX + 0.5f, mapMaxX - 0.5f),
		ofClamp(p.y, 0.35f, wallHeight - 0.25f),
		ofClamp(p.z, mapMinZ + 0.5f, mapMaxZ - 0.5f)
	};
}

void ofApp::moveSelected(const glm::vec3& delta) {
	if (selection == Selection::Listener) listenerPos = clampToMap(listenerPos + delta);
	else sourcePos = clampToMap(sourcePos + delta);
}

// -----------------------------------------------------------------------------
void ofApp::update() {
	if (!ok) return;

	const float dt = ofGetLastFrameTime();
	const float step = moveSpeed * dt;

	glm::vec3 ahead = cam.getLookAtDir();
	ahead.y = 0;
	if (glm::length2(ahead) > 1e-6f) ahead = glm::normalize(ahead);
	else ahead = glm::vec3(0, 0, 1);
	glm::vec3 right = glm::normalize(glm::cross(ahead, glm::vec3(0, 1, 0)));

	glm::vec3 delta(0);
	if (keys['w'] || keys['W'] || keys[OF_KEY_UP])    delta += ahead * step;
	if (keys['s'] || keys['S'] || keys[OF_KEY_DOWN])  delta -= ahead * step;
	if (keys['a'] || keys['A'] || keys[OF_KEY_LEFT])  delta -= right * step;
	if (keys['d'] || keys['D'] || keys[OF_KEY_RIGHT]) delta += right * step;
	if (keys['q'] || keys['Q']) delta.y -= step;
	if (keys['e'] || keys['E']) delta.y += step;
	if (glm::length2(delta) > 0) moveSelected(delta);

	glm::vec3 toSrc = sourcePos - listenerPos;
	toSrc.y = 0;
	if (glm::length2(toSrc) > 1e-4f) listenerAhead = glm::normalize(toSrc);

	runDirectSim();
}

// -----------------------------------------------------------------------------
void ofApp::drawLabels() const {
	if (!showLabels) return;
	ofSetColor(255, 240, 180);
	auto label = [](glm::vec3 p, const std::string& s) {
		ofDrawBitmapString(s, p + glm::vec3(0, 0.3f, 0));
	};
	label({ 0, 0.2f, -22 }, "LONG (T)");
	label({ 0, 0.2f,  2 }, "CAR / CORNER");
	label({ 0, 0.2f, 13 }, "A SITE / PLANT");
	label({ -3.5f, 2.2f, 14 }, "GOOSE");
	label({ 8.5f, 3.0f, 14.5f }, "PLATFORM");
	label({ 8, 0.2f, 2 }, "SHORT");
	label({ 2, 0.2f, 19 }, "CT BACK");
}

// -----------------------------------------------------------------------------
void ofApp::draw() {
	ofBackground(35, 38, 42);

	cam.begin();

	// Sand-colored ground
	ofSetColor(175, 160, 115);
	ground.draw();
	ofSetColor(140, 125, 90, 100);
	ofPushMatrix();
	ofTranslate((mapMinX + mapMaxX) * 0.5f, 0.01f, (mapMinZ + mapMaxZ) * 0.5f);
	ofRotateXDeg(90);
	ofDrawGridPlane(55, 28, false);
	ofPopMatrix();

	// Walls / boxes
	for (auto& w : walls) {
		ofSetColor(w.color);
		w.mesh.draw();
		ofSetColor(20, 18, 15, 100);
		w.mesh.drawWireframe();
	}

	drawLabels();

	if (showProbes) {
		ofSetColor(70, 160, 255, 90);
		for (const auto& p : probeSpheres) {
			ofDrawSphere(p.center.x, p.center.y, p.center.z, 0.18f);
		}
	}

	// LOS listener → source
	{
		ofSetLineWidth(2);
		ofSetColor(ofColor::fromHsb(ofMap(lastOcclusion, 0, 1, 95, 0, true), 210, 255));
		ofDrawLine(listenerPos, sourcePos);
		ofSetLineWidth(1);
	}

	// Listener
	{
		const bool sel = (selection == Selection::Listener);
		ofSetColor(sel ? ofColor(80, 230, 255) : ofColor(40, 160, 200));
		ofDrawSphere(listenerPos, sel ? 0.35f : 0.28f);
		ofSetColor(255, 255, 90);
		ofDrawArrow(listenerPos, listenerPos + listenerAhead * 1.0f, 0.1f);
		ofSetColor(255);
		ofDrawBitmapString("LISTENER (you)", listenerPos + glm::vec3(0, 0.55f, 0));
	}

	// Emitter
	{
		const bool sel = (selection == Selection::Source);
		ofSetColor(sel ? ofColor(255, 130, 50) : ofColor(200, 70, 35));
		ofDrawSphere(sourcePos, sel ? 0.35f : 0.28f);
		ofNoFill();
		ofPushMatrix();
		ofTranslate(sourcePos);
		ofRotateXDeg(90);
		ofDrawCircle(0, 0, 0.45f + 0.1f * sinf(ofGetElapsedTimef() * 7.0f));
		ofPopMatrix();
		ofFill();
		ofSetColor(255);
		ofDrawBitmapString("EMITTER (gunfire)", sourcePos + glm::vec3(0, 0.55f, 0));
	}

	cam.end();
	drawHelp();
}

// -----------------------------------------------------------------------------
void ofApp::drawHelp() const {
	const std::string selName = (selection == Selection::Listener) ? "LISTENER" : "EMITTER";
	const float dist = glm::distance(listenerPos, sourcePos);
	ofDrawBitmapStringHighlight(
		"de_dust2 A-site + Long (simplified walls for occlusion)\n"
		"Selected: " + selName + "  [Tab]\n"
		"WASD/arrows move · Q/E height · Shift+LMB drag place\n"
		"[R] reset actors + camera · [P] probes · [L] labels\n"
		"dist=" + ofToString(dist, 1) + "m"
		"  atten=" + ofToString(lastAttenuation, 2)
		+ "  occ=" + ofToString(lastOcclusion, 2)
		+ "  walls=" + ofToString((int)walls.size())
		+ "  probes=" + ofToString((int)probeSpheres.size()),
		12, 18);
}

// -----------------------------------------------------------------------------
void ofApp::audioOut(ofSoundBuffer& buffer) {
	if (!ok) { buffer.set(0); return; }

	// Slightly lower pitch when far / occluded for feedback
	const float baseHz = 260.0f;
	const float hz = baseHz * ofMap(lastAttenuation, 0.05f, 1.0f, 0.75f, 1.0f, true);
	monoIn.fillSine(hz, (float)audioSettings.samplingRate, 0.48f, phase);

	IPLSimulationOutputs outputs{};
	simSource.getOutputs(IPL_SIMULATIONFLAGS_DIRECT, &outputs);

	IPLDirectEffectParams dp = outputs.direct;
	dp.flags = static_cast<IPLDirectEffectFlags>(
		IPL_DIRECTEFFECTFLAGS_APPLYDISTANCEATTENUATION |
		IPL_DIRECTEFFECTFLAGS_APPLYAIRABSORPTION |
		IPL_DIRECTEFFECTFLAGS_APPLYOCCLUSION |
		IPL_DIRECTEFFECTFLAGS_APPLYTRANSMISSION);
	dp.transmissionType = IPL_TRANSMISSIONTYPE_FREQINDEPENDENT;
	// Walls still leak a little energy when occluded
	if (dp.occlusion > 0.5f) {
		for (int b = 0; b < IPL_NUM_BANDS; ++b) {
			if (dp.transmission[b] < 0.08f) dp.transmission[b] = 0.08f;
		}
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
	if (key == 'l' || key == 'L') showLabels = !showLabels;
	if (key == 'r' || key == 'R') {
		resetActors();
		resetCamera();
	}
}

void ofApp::keyReleased(int key) {
	if (key >= 0 && key < 512) keys[key] = false;
}

bool ofApp::rayPlaneY(const glm::vec2& screen, float y, glm::vec3& hit) const {
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
	if (!ofGetKeyPressed(OF_KEY_SHIFT)) return;
	dragging = true;
	cam.disableMouseInput();
	glm::vec3 hit;
	const float placeY = (selection == Selection::Listener) ? listenerPos.y : sourcePos.y;
	if (rayPlaneY(glm::vec2(x, y), placeY, hit)) {
		hit.y = placeY;
		if (selection == Selection::Listener) listenerPos = clampToMap(hit);
		else sourcePos = clampToMap(hit);
	}
}

void ofApp::mouseDragged(int x, int y, int button) {
	if (!dragging || button != OF_MOUSE_BUTTON_LEFT) return;
	glm::vec3 hit;
	const float placeY = (selection == Selection::Listener) ? listenerPos.y : sourcePos.y;
	if (rayPlaneY(glm::vec2(x, y), placeY, hit)) {
		hit.y = placeY;
		if (selection == Selection::Listener) listenerPos = clampToMap(hit);
		else sourcePos = clampToMap(hit);
	}
}

void ofApp::mouseReleased(int x, int y, int button) {
	if (button == OF_MOUSE_BUTTON_LEFT && dragging) {
		dragging = false;
		cam.enableMouseInput();
	}
}
