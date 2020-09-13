#include "pge/scene/RenderScene.h"

#include "pge/sceneobjects/SceneObjectEntryPointBlockGame.h"
#include "pge/sceneobjects/SceneObjectEntryPointSurf.h"
#include "pge/sceneobjects/SceneObjectEntryPointCartPole.h"
#include "pge/sceneobjects/SceneObjectEntryPointQuadruped.h"
#include "pge/sceneobjects/SceneObjectEntryPointTennis.h"

#include "pge/sceneobjects/input/SceneObjectBufferedInput.h"

#include "pge/rendering/lighting/SceneObjectLighting.h"
#include "pge/rendering/model/SceneObjectStaticModelBatcher.h"

#include "pge/util/Math.h"

#include <fstream>

#define PGE_FIXED_TIMESTEP 0.017f

int main(int argc, char *argv[]) {
	int width = 640;
	int height = 480;

	std::shared_ptr<pge::SceneObject> entryPoint;

	entryPoint.reset(new SceneObjectEntryPointCartPole());
        
	bool show = true;

	// Command line arguments
	/*if (argc != 5) {
		std::cerr << "Invalid number of command line arguments. Requires 4 arguments." << std::endl;

		return 1;
	}
	else {
		std::string s1 = argv[1];
		
		if (s1 == "BlockGame-v0") {
			entryPoint.reset(new SceneObjectEntryPointBlockGame());
		}
		else if (s1 == "CartPole3D-v0") {
			entryPoint.reset(new SceneObjectEntryPointCartPole());
		}
		else if (s1 == "Quadruped3D-v0") {
			entryPoint.reset(new SceneObjectEntryPointQuadruped());
		}
		else if (s1 == "Surf-v0") {
			entryPoint.reset(new SceneObjectEntryPointSurf());
		}
		else if (s1 == "Tennis3D-v0") {
			entryPoint.reset(new SceneObjectEntryPointTennis());
		}
		else {
			std::cerr << "Invalid environment name (\"" << s1 << "\" given)" << std::endl;

			return 1;
		}

		std::string s2 = argv[2];

		if (s2 == "show") {
			show = true;
		}
		else if (s2 == "hide") {
			show = false;
		}

		std::string s3 = argv[3];
		std::string s4 = argv[4];

		width = std::stoi(s3);
		height = std::stoi(s4);
	}*/

	sf::Window window;
	sf::ContextSettings settings;

	settings.majorVersion = 4; // was 4
	settings.minorVersion = 1; // was 4
	settings.stencilBits = 0;
	settings.antialiasingLevel = 0;
    //settings.attributeFlags = sf::ContextSettings::Core;

	window.create(sf::VideoMode(width, height), "pge", sf::Style::Default, settings);

	window.setVerticalSyncEnabled(false);
	window.setFramerateLimit(0);

	if (ogl_LoadFunctions() == ogl_LOAD_FAILED) {
		std::cerr << "Could not load OpenGL extensions!" << std::endl;
		abort();
	}

	// -------------------------------- OpenGL Setup --------------------------------
    
	pge::sfmloglSetup();
   
	glViewport(0, 0, window.getSize().x, window.getSize().y);

	pge::checkForGLError();

	// -------------------------------- Scene Setup --------------------------------

	std::unique_ptr<pge::RenderScene> scene(new pge::RenderScene());

	scene->renderingEnabled = show;

	//scene->randomGenerator.seed(time(nullptr));

	{
		std::shared_ptr<pge::Shader> gBufferRender(new pge::Shader());
		std::shared_ptr<pge::Shader> gBufferRenderNormal(new pge::Shader());
		std::shared_ptr<pge::Shader> gBufferRenderHeightNormal(new pge::Shader());
		std::shared_ptr<pge::Texture2D> whiteTexture(new pge::Texture2D());

		gBufferRender->createAsset("NONE resources/shaders/gbufferrender/gBufferRender.vert resources/shaders/gbufferrender/gBufferRender.frag");
		gBufferRenderNormal->createAsset("NONE resources/shaders/gbufferrender/gBufferRenderBump.vert resources/shaders/gbufferrender/gBufferRenderBump.frag");
		gBufferRenderHeightNormal->createAsset("NONE resources/shaders/gbufferrender/gBufferRenderParallax.vert resources/shaders/gbufferrender/gBufferRenderParallax.frag");
		whiteTexture->createAsset("resources/shaders/white.png");

		scene->createRenderScene(8, pge::AABB3D(pge::Vec3f(-1.0f, -1.0f, -1.0f), pge::Vec3f(1.0f, 1.0f, 1.0f)), &window,
			gBufferRender, gBufferRenderNormal, gBufferRenderHeightNormal, whiteTexture);

		scene->logicCamera.projectionMatrix = pge::Matrix4x4f::perspectiveMatrix(pge::piOver4, static_cast<float>(window.getSize().x) / static_cast<float>(window.getSize().y), 0.1f, 10000.0f);
		scene->logicCamera.position = pge::Vec3f(1.5f, 1.5f, 1.5f);
		scene->logicCamera.rotation = pge::Quaternion::getFromMatrix(pge::Matrix4x4f::cameraDirectionMatrix(-scene->logicCamera.position.normalized(), pge::Vec3f(0.0f, 1.0f, 0.0f)));
	}

	// -------------------------------- Lighting --------------------------------

	{
		std::shared_ptr<pge::Shader> ambientLightShader(new pge::Shader());
		std::shared_ptr<pge::Shader> pointLightShader(new pge::Shader());
		std::shared_ptr<pge::Shader> pointLightShadowedShader(new pge::Shader());
		std::shared_ptr<pge::Shader> spotLightShader(new pge::Shader());
		std::shared_ptr<pge::Shader> directionalLightShader(new pge::Shader());
		std::shared_ptr<pge::Shader> directionalLightShadowedShader(new pge::Shader());
		std::shared_ptr<pge::Shader> emissiveRenderShader(new pge::Shader());
		std::shared_ptr<pge::Shader> depthRenderShader(new pge::Shader());

		std::shared_ptr<pge::StaticPositionModel> sphereModel(new pge::StaticPositionModel());
		std::shared_ptr<pge::StaticPositionModel> coneModel(new pge::StaticPositionModel());

		ambientLightShader->createAsset("NONE resources/shaders/noTransformVertex.vert resources/shaders/noTransformFragment.frag");
		pointLightShader->createAsset("NONE resources/shaders/positionOnlyVertex.vert resources/shaders/light/unshadowed/point.frag");
		pointLightShadowedShader->createAsset("NONE resources/shaders/positionOnlyVertex.vert resources/shaders/light/shadowed/point.frag");
		spotLightShader->createAsset("NONE resources/shaders/positionOnlyVertex.vert resources/shaders/light/unshadowed/spot.frag");
		directionalLightShader->createAsset("NONE resources/shaders/noTransformVertex.vert resources/shaders/light/unshadowed/directional.frag");
		directionalLightShadowedShader->createAsset("NONE resources/shaders/noTransformVertex.vert resources/shaders/light/shadowed/directionalShadowed.frag");
		emissiveRenderShader->createAsset("NONE resources/shaders/noTransformVertex.vert resources/shaders/light/emissiveRender.frag");
		depthRenderShader->createAsset("NONE resources/shaders/defaultVertex.vert resources/shaders/depthRender.frag");

		pge::AABB3D aabb;

		sphereModel->loadFromOBJ("resources/shaders/models/icosphere_lowRes.obj", aabb, true, true);
		coneModel->loadFromOBJ("resources/shaders/models/cone.obj", aabb, true, true);

		std::shared_ptr<pge::Asset> assetNoise;

		scene->getAssetManager("tex2D", pge::Texture2D::assetFactory)->getAsset("resources/textures/noise.bmp", assetNoise);

		std::shared_ptr<pge::Texture2D> noiseMap = std::static_pointer_cast<pge::Texture2D>(assetNoise);

		std::shared_ptr<pge::SceneObjectLighting> lighting(new pge::SceneObjectLighting());

		scene->addNamed(lighting, "lighting");

		lighting->create(ambientLightShader,
			pointLightShader, pointLightShadowedShader,
			spotLightShader,
			directionalLightShader, directionalLightShadowedShader,
			emissiveRenderShader, depthRenderShader,
			sphereModel, coneModel,
			noiseMap);
	}

	// --------------------------------- Input -----------------------------------

	pge::SceneObjectRef bufferedInputRef;

	{
		std::shared_ptr<pge::SceneObjectBufferedInput> bufferedInput(new pge::SceneObjectBufferedInput());

		scene->addNamed(bufferedInput, "buffIn");

		bufferedInputRef = *bufferedInput;
	}

	// -------------------------------- Batching ---------------------------------

	{
		std::shared_ptr<pge::SceneObjectStaticModelBatcher> staticModelBatcher(new pge::SceneObjectStaticModelBatcher());

		scene->addNamed(staticModelBatcher, "smb", false);
	}

	// -------------------------------- Game Loop --------------------------------

	// Add entry point
	{
		scene->add(entryPoint);
	}

	bool quit = false;

	sf::Clock clock;

	float dt = 0.0f;

	while (!quit) {
		{
			sf::Event windowEvent;

			pge::SceneObjectBufferedInput* pBufferedInput = static_cast<pge::SceneObjectBufferedInput*>(bufferedInputRef.get());

			while (window.pollEvent(windowEvent)) {
				pBufferedInput->events.push_back(windowEvent);

				switch (windowEvent.type) {
				case sf::Event::Closed:
					quit = true;

					break;
				}
			}

			//if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
			//	quit = true;

			if (scene->close)
				quit = true;
		}

#ifdef PGE_FIXED_TIMESTEP
		scene->frame(PGE_FIXED_TIMESTEP);
#else
		scene->frame(dt);
#endif

		glFlush();

		window.display();

		dt = clock.getElapsedTime().asSeconds();
		clock.restart();
	}

	scene.reset();

	window.close();

	return 0;
}
