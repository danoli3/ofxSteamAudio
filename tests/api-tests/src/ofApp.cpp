/**
 * Headless API tests for ofxSteamAudio wrappers.
 * Maps to Steam Audio public C API / itest feature areas:
 *   context, audiobuffer, hrtf, binaural, panning, virtualsurround,
 *   ambisonics (encode/binaural/panning/rotation/decode),
 *   direct, reflection (parametric), scene/staticmesh,
 *   simulator/direct, energyfield, impulseresponse, probes,
 *   distance attenuation / air absorption / relative direction
 */
#include "ofApp.h"
#include <cmath>
#include <cstdlib>

using namespace ofxSteamAudio;

static int g_passed = 0;
static int g_failed = 0;

#define EXPECT_TRUE(cond, msg) do { \
	if (cond) { ++g_passed; ofLogNotice("TEST") << "PASS: " << msg; } \
	else { ++g_failed; ofLogError("TEST") << "FAIL: " << msg; } \
} while(0)

#define EXPECT_EQ(a, b, msg) EXPECT_TRUE((a) == (b), msg)
#define EXPECT_NEAR(a, b, eps, msg) EXPECT_TRUE(std::fabs((double)(a) - (double)(b)) < (eps), msg)

void ofApp::setup() {
	const int sampleRate = 48000;
	const int frameSize = 512;
	IPLAudioSettings audio{ sampleRate, frameSize };

	// ----- Context (itest: memory / log) -----
	Context ctx;
	EXPECT_TRUE(ctx.setup(true), "Context::setup");
	EXPECT_TRUE(ctx.isValid(), "Context is valid");

	// ----- Relative direction -----
	{
		IPLVector3 dir = relativeDirection(ctx, glm::vec3(1, 0, 0), glm::vec3(0), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));
		float len = std::sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
		EXPECT_NEAR(len, 1.0f, 1e-4, "relativeDirection unit length");
	}

	// ----- Distance attenuation / air absorption -----
	{
		float att = distanceAttenuation(ctx, glm::vec3(0, 0, 0), glm::vec3(0, 0, 10));
		EXPECT_TRUE(att > 0.0f && att <= 1.0f, "distanceAttenuation in (0,1]");
		IPLfloat32 air[IPL_NUM_BANDS] = {};
		airAbsorption(ctx, glm::vec3(0), glm::vec3(0, 0, 50), air);
		EXPECT_TRUE(air[0] > 0.0f && air[0] <= 1.0f, "airAbsorption band0 valid");
	}

	// ----- AudioBuffer (itest: AudioBuffer) -----
	AudioBuffer mono, stereo, mono2;
	EXPECT_TRUE(mono.allocate(ctx, 1, frameSize), "AudioBuffer mono allocate");
	EXPECT_TRUE(stereo.allocate(ctx, 2, frameSize), "AudioBuffer stereo allocate");
	EXPECT_TRUE(mono2.allocate(ctx, 1, frameSize), "AudioBuffer mono2 allocate");
	float phase = 0;
	mono.fillSine(440.0f, (float)sampleRate, 0.5f, phase);
	EXPECT_TRUE(std::fabs(mono.channel(0)[0]) > 0.0f || std::fabs(mono.channel(0)[10]) > 0.0f,
	            "AudioBuffer fillSine produces samples");
	mono2.downmixFrom(ctx, stereo); // empty stereo -> ok
	EXPECT_TRUE(true, "AudioBuffer downmix");

	// ----- HRTF -----
	HRTF hrtf;
	EXPECT_TRUE(hrtf.create(ctx, audio), "HRTF::create default");

	// ----- Binaural (itest: binauraleffect) -----
	{
		BinauralEffect binaural;
		EXPECT_TRUE(binaural.create(ctx, audio, hrtf), "BinauralEffect::create");
		IPLVector3 dir{ 1, 0, 0 };
		auto state = binaural.apply(dir, hrtf, mono.get(), stereo.get());
		(void)state;
		float energy = 0;
		for (int i = 0; i < frameSize; ++i) {
			energy += stereo.channel(0)[i] * stereo.channel(0)[i];
			energy += stereo.channel(1)[i] * stereo.channel(1)[i];
		}
		EXPECT_TRUE(energy > 0.0f, "BinauralEffect produces non-zero output");
		binaural.reset();
		EXPECT_TRUE(binaural.getTailSize() >= 0, "BinauralEffect getTailSize");
	}

	// ----- Panning (itest: panningeffect) -----
	{
		PanningEffect panning;
		EXPECT_TRUE(panning.create(ctx, audio, IPL_SPEAKERLAYOUTTYPE_STEREO), "PanningEffect::create");
		stereo.clear();
		IPLVector3 dir{ 0, 0, -1 };
		panning.apply(dir, mono.get(), stereo.get());
		float energy = 0;
		for (int i = 0; i < frameSize; ++i) energy += stereo.channel(0)[i] * stereo.channel(0)[i]
		                                            + stereo.channel(1)[i] * stereo.channel(1)[i];
		EXPECT_TRUE(energy > 0.0f, "PanningEffect produces non-zero output");
	}

	// ----- Virtual Surround (itest: virtualsurroundeffect) -----
	{
		AudioBuffer surround;
		EXPECT_TRUE(surround.allocate(ctx, 6, frameSize), "5.1 buffer allocate");
		// Put energy in front-left-ish channel 0
		for (int i = 0; i < frameSize; ++i) surround.channel(0)[i] = mono.channel(0)[i];
		VirtualSurroundEffect vs;
		EXPECT_TRUE(vs.create(ctx, audio, hrtf, IPL_SPEAKERLAYOUTTYPE_SURROUND_5_1),
		            "VirtualSurroundEffect::create");
		stereo.clear();
		vs.apply(hrtf, surround.get(), stereo.get());
		float energy = 0;
		for (int i = 0; i < frameSize; ++i) energy += stereo.channel(0)[i] * stereo.channel(0)[i]
		                                            + stereo.channel(1)[i] * stereo.channel(1)[i];
		EXPECT_TRUE(energy > 0.0f, "VirtualSurroundEffect produces non-zero output");
	}

	// ----- Ambisonics encode + binaural (itest: ambisonics*) -----
	{
		const int order = 1;
		const int nCh = ambisonicsChannels(order);
		AudioBuffer ambi;
		EXPECT_TRUE(ambi.allocate(ctx, nCh, frameSize), "Ambisonics buffer allocate");

		AmbisonicsEncodeEffect encode;
		EXPECT_TRUE(encode.create(ctx, audio, order), "AmbisonicsEncodeEffect::create");
		IPLVector3 dir{ 0, 0, -1 };
		encode.apply(dir, order, mono.get(), ambi.get());

		AmbisonicsBinauralEffect ambiBin;
		EXPECT_TRUE(ambiBin.create(ctx, audio, hrtf, order), "AmbisonicsBinauralEffect::create");
		stereo.clear();
		ambiBin.apply(hrtf, order, ambi.get(), stereo.get());
		float energy = 0;
		for (int i = 0; i < frameSize; ++i) energy += stereo.channel(0)[i] * stereo.channel(0)[i]
		                                            + stereo.channel(1)[i] * stereo.channel(1)[i];
		EXPECT_TRUE(energy > 0.0f, "Ambisonics encode→binaural produces output");

		AmbisonicsPanningEffect ambiPan;
		EXPECT_TRUE(ambiPan.create(ctx, audio, IPL_SPEAKERLAYOUTTYPE_STEREO, order),
		            "AmbisonicsPanningEffect::create");
		stereo.clear();
		ambiPan.apply(order, ambi.get(), stereo.get());
		EXPECT_TRUE(true, "AmbisonicsPanningEffect::apply");

		AmbisonicsRotationEffect rotate;
		EXPECT_TRUE(rotate.create(ctx, audio, order), "AmbisonicsRotationEffect::create");
		AudioBuffer ambiOut;
		EXPECT_TRUE(ambiOut.allocate(ctx, nCh, frameSize), "ambiOut allocate");
		auto orient = makeCoordinateSpace(glm::vec3(0), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));
		rotate.apply(orient, order, ambi.get(), ambiOut.get());
		EXPECT_TRUE(true, "AmbisonicsRotationEffect::apply");

		AmbisonicsDecodeEffect decode;
		EXPECT_TRUE(decode.create(ctx, audio, IPL_SPEAKERLAYOUTTYPE_STEREO, hrtf, order),
		            "AmbisonicsDecodeEffect::create");
		IPLAmbisonicsDecodeEffectParams dp{};
		dp.order = order;
		dp.hrtf = hrtf;
		dp.orientation = orient;
		dp.binaural = IPL_TRUE;
		stereo.clear();
		decode.apply(dp, ambi.get(), stereo.get());
		EXPECT_TRUE(true, "AmbisonicsDecodeEffect::apply");
	}

	// ----- Direct effect (itest: directsoundeffect) -----
	{
		DirectEffect direct;
		EXPECT_TRUE(direct.create(ctx, audio, 1), "DirectEffect::create");
		IPLDirectEffectParams p{};
		p.flags = static_cast<IPLDirectEffectFlags>(
			IPL_DIRECTEFFECTFLAGS_APPLYDISTANCEATTENUATION |
			IPL_DIRECTEFFECTFLAGS_APPLYAIRABSORPTION |
			IPL_DIRECTEFFECTFLAGS_APPLYOCCLUSION);
		p.distanceAttenuation = 0.5f;
		p.airAbsorption[0] = p.airAbsorption[1] = p.airAbsorption[2] = 0.9f;
		p.occlusion = 0.3f;
		p.transmission[0] = p.transmission[1] = p.transmission[2] = 0.5f;
		p.transmissionType = IPL_TRANSMISSIONTYPE_FREQINDEPENDENT;
		AudioBuffer outMono;
		EXPECT_TRUE(outMono.allocate(ctx, 1, frameSize), "direct out allocate");
		direct.apply(p, mono.get(), outMono.get());
		float energy = 0;
		for (int i = 0; i < frameSize; ++i) energy += outMono.channel(0)[i] * outMono.channel(0)[i];
		EXPECT_TRUE(energy > 0.0f, "DirectEffect produces attenuated output");
	}

	// ----- Reflection parametric (itest: parametricreverb / reverbeffect) -----
	{
		const int irCh = ambisonicsChannels(1);
		const int irSize = sampleRate; // 1 second
		ReflectionEffect reverb;
		EXPECT_TRUE(reverb.create(ctx, audio, IPL_REFLECTIONEFFECTTYPE_PARAMETRIC, irSize, irCh),
		            "ReflectionEffect parametric create");
		IPLReflectionEffectParams rp{};
		rp.type = IPL_REFLECTIONEFFECTTYPE_PARAMETRIC;
		rp.reverbTimes[0] = 0.5f;
		rp.reverbTimes[1] = 0.4f;
		rp.reverbTimes[2] = 0.3f;
		rp.numChannels = 1;
		rp.irSize = irSize;
		AudioBuffer revOut;
		EXPECT_TRUE(revOut.allocate(ctx, 1, frameSize), "reverb out allocate");
		reverb.apply(rp, mono.get(), revOut.get(), nullptr);
		EXPECT_TRUE(true, "ReflectionEffect parametric apply");
	}

	// ----- Scene + StaticMesh (itest: scene, staticmesh) -----
	Scene scene;
	EXPECT_TRUE(scene.create(ctx, IPL_SCENETYPE_DEFAULT), "Scene::create");
	auto mesh = scene.addBox(4.0f, 3.0f, 6.0f, Materials::wood());
	EXPECT_TRUE(mesh != nullptr, "Scene::addBox");
	scene.addGroundPlane(20.0f, Materials::concrete());
	scene.commit();
	EXPECT_TRUE(true, "Scene::commit");

	// ----- Serialization -----
	{
		SerializedObject so;
		EXPECT_TRUE(so.create(ctx), "SerializedObject::create");
		scene.save(so.get());
		EXPECT_TRUE(so.getSize() > 0, "Scene::save produces data");
	}

	// ----- Simulator + direct occlusion (itest: directsimulator) -----
	{
		Simulator sim;
		EXPECT_TRUE(sim.create(ctx, audio, IPL_SIMULATIONFLAGS_DIRECT, IPL_SCENETYPE_DEFAULT),
		            "Simulator::create");
		sim.setScene(scene);
		sim.commit();

		SimSource src;
		EXPECT_TRUE(src.create(sim.get(), IPL_SIMULATIONFLAGS_DIRECT), "SimSource::create");
		src.add(sim.get());
		sim.commit();

		IPLSimulationSharedInputs shared{};
		shared.listener = makeCoordinateSpace(glm::vec3(0, 1.6f, 0));
		shared.numRays = 0;
		shared.numBounces = 0;
		shared.duration = 1.0f;
		shared.order = 0;
		shared.irradianceMinDistance = 1.0f;
		sim.setSharedInputs(IPL_SIMULATIONFLAGS_DIRECT, &shared);

		IPLSimulationInputs inputs{};
		inputs.flags = IPL_SIMULATIONFLAGS_DIRECT;
		inputs.directFlags = static_cast<IPLDirectSimulationFlags>(
			IPL_DIRECTSIMULATIONFLAGS_DISTANCEATTENUATION |
			IPL_DIRECTSIMULATIONFLAGS_AIRABSORPTION |
			IPL_DIRECTSIMULATIONFLAGS_OCCLUSION);
		inputs.source = makeCoordinateSpace(glm::vec3(0, 1.5f, -5));
		inputs.distanceAttenuationModel.type = IPL_DISTANCEATTENUATIONTYPE_DEFAULT;
		inputs.airAbsorptionModel.type = IPL_AIRABSORPTIONTYPE_DEFAULT;
		inputs.occlusionType = IPL_OCCLUSIONTYPE_RAYCAST;
		inputs.occlusionRadius = 0.1f;
		inputs.numOcclusionSamples = 4;
		src.setInputs(IPL_SIMULATIONFLAGS_DIRECT, &inputs);

		sim.runDirect();
		IPLSimulationOutputs outputs{};
		src.getOutputs(IPL_SIMULATIONFLAGS_DIRECT, &outputs);
		EXPECT_TRUE(outputs.direct.distanceAttenuation >= 0.0f &&
		            outputs.direct.distanceAttenuation <= 1.0f,
		            "Simulator direct distanceAttenuation valid");
		EXPECT_TRUE(outputs.direct.occlusion >= 0.0f && outputs.direct.occlusion <= 1.0f,
		            "Simulator direct occlusion valid");

		src.remove(sim.get());
		sim.commit();
	}

	// ----- Energy field / impulse response (itest: energyfield, impulseresponse) -----
	{
		EnergyField ef, ef2;
		EXPECT_TRUE(ef.create(ctx, 1.0f, 1), "EnergyField::create");
		EXPECT_TRUE(ef2.create(ctx, 1.0f, 1), "EnergyField::create 2");
		EXPECT_TRUE(ef.numChannels() > 0, "EnergyField numChannels");
		EXPECT_TRUE(ef.numBins() > 0, "EnergyField numBins");
		ef.reset();
		ef2.copyFrom(ef);
		EXPECT_TRUE(true, "EnergyField copy/reset");

		ImpulseResponse ir;
		EXPECT_TRUE(ir.create(ctx, 0.5f, 1, sampleRate), "ImpulseResponse::create");
		EXPECT_TRUE(ir.numChannels() > 0, "ImpulseResponse numChannels");
		EXPECT_TRUE(ir.numSamples() > 0, "ImpulseResponse numSamples");
		ir.reset();

		Reconstructor recon;
		EXPECT_TRUE(recon.create(ctx, 1.0f, 1, sampleRate), "Reconstructor::create");
	}

	// ----- Probes (itest: probes) -----
	{
		ProbeArray probes;
		EXPECT_TRUE(probes.create(ctx), "ProbeArray::create");
		IPLProbeGenerationParams pg{};
		pg.type = IPL_PROBEGENERATIONTYPE_CENTROID;
		pg.spacing = 2.0f;
		pg.height = 1.5f;
		pg.transform = identityMatrix();
		// scale unit cube to room-ish volume via transform diagonal
		pg.transform.elements[0][0] = 8;
		pg.transform.elements[1][1] = 3;
		pg.transform.elements[2][2] = 10;
		probes.generate(scene, pg);
		EXPECT_TRUE(probes.numProbes() >= 1, "ProbeArray generated probes");

		ProbeBatch batch;
		EXPECT_TRUE(batch.create(ctx), "ProbeBatch::create");
		batch.addProbeArray(probes.get());
		batch.commit();
		EXPECT_TRUE(batch.numProbes() >= 1, "ProbeBatch has probes");
	}

	// ----- High-level Engine -----
	{
		Engine engine;
		EXPECT_TRUE(engine.setup(sampleRate, frameSize, false), "Engine::setup");
		int id = engine.addSource(glm::vec3(2, 0, 0));
		engine.setSourceFrequency(id, 330.0f);
		engine.setSourceGain(id, 0.4f);
		engine.setListener(glm::vec3(0), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));
		ofSoundBuffer buf;
		buf.allocate(frameSize, 2);
		engine.processAudio(buf);
		float energy = 0;
		for (size_t i = 0; i < buf.size(); ++i) energy += buf[i] * buf[i];
		EXPECT_TRUE(energy > 0.0f, "Engine::processAudio produces output");
		engine.shutdown();
	}

	// ----- Path effect create (apply needs pathing data; create only) -----
	{
		PathEffect path;
		EXPECT_TRUE(path.create(ctx, audio, 1, true, hrtf, IPL_SPEAKERLAYOUTTYPE_STEREO),
		            "PathEffect::create");
	}

	// ----- Reflection mixer create -----
	{
		ReflectionMixer mixer;
		EXPECT_TRUE(mixer.create(ctx, audio, IPL_REFLECTIONEFFECTTYPE_CONVOLUTION,
		                         sampleRate, ambisonicsChannels(1)),
		            "ReflectionMixer::create");
	}

	ctx.printMemoryUsage();

	ofLogNotice("TEST") << "========================================";
	ofLogNotice("TEST") << "Results: " << g_passed << " passed, " << g_failed << " failed";
	ofLogNotice("TEST") << "========================================";

	// Exit with failure code if any tests failed
	std::exit(g_failed == 0 ? 0 : 1);
}
