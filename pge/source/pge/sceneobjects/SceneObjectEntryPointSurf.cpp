#include <pge/sceneobjects/SceneObjectEntryPointSurf.h>

#include <pge/sceneobjects/SceneObjectProp.h>
#include <pge/sceneobjects/SceneObjectPlayer.h>
#include <pge/sceneobjects/SceneObjectBox.h>
#include <pge/sceneobjects/SceneObjectSurf.h>

#include <pge/rendering/voxel/VoxelTerrain.h>
#include <pge/rendering/voxel/TerrainGenerator.h>
#include <pge/rendering/lighting/SceneObjectDirectionalLightShadowed.h>
#include <pge/rendering/lighting/SceneObjectPointLightShadowed.h>

#include <pge/rendering/imageeffects/SceneObjectEffectBuffer.h>
#include <pge/rendering/imageeffects/SceneObjectSSAO.h>
#include <pge/rendering/imageeffects/SceneObjectLightScattering.h>
#include <pge/rendering/imageeffects/SceneObjectDepthOfField.h>
#include <pge/rendering/imageeffects/SceneObjectSSR.h>
#include <pge/rendering/imageeffects/SceneObjectFXAA.h>
#include <pge/rendering/imageeffects/SceneObjectFog.h>

#include <pge/sceneobjects/SceneObjectFloatingCamera.h>
#include <pge/sceneobjects/SceneObjectOrbitCamera.h>

#include <pge/system/SoftwareImage2D.h>

void SceneObjectEntryPointSurf::onAdd() {
	// Physics

	std::shared_ptr<pge::SceneObjectPhysicsWorld> physicsWorld(new pge::SceneObjectPhysicsWorld());

	getRenderScene()->addNamed(physicsWorld, "physWrld");

	pge::SceneObjectRef lighting = getScene()->getNamed("lighting");

	pge::SceneObjectLighting* pLighting = static_cast<pge::SceneObjectLighting*>(lighting.get());

	pLighting->_ambientLight = pge::Vec3f(0.01f, 0.01f, 0.01f);

	/*std::shared_ptr<pge::SceneObjectDirectionalLightShadowed> light(new pge::SceneObjectDirectionalLightShadowed());

	getRenderScene()->add(light);

	light->create(pLighting, 2, 2048, 0.5f, 100.0f, 0.6f);

	light->setDirection(pge::Vec3f(-0.4f, -1.0f, 0.6f).normalized());

	light->setColor(pge::Vec3f(1.0f, 1.0f, 1.0f));*/

	// GUI

	/*std::shared_ptr<SceneObjectGUI> gui(new SceneObjectGUI());

	getRenderScene()->addNamed(gui, "gui", false);

	gui->_layer = 2.0f;*/

	// Control

	std::shared_ptr<SceneObjectOrbitCamera> camera(new SceneObjectOrbitCamera());

	getRenderScene()->add(camera, false);

	// Map

	/*std::shared_ptr<pge::Map3DWS> map(new pge::Map3DWS());

	map->_settings._pScene = getScene();

	map->createAsset("resources/maps/horrorMap.3dw");

	pge::addMapLights(*map, getScene());*/

	std::shared_ptr<SceneObjectProp> prop(new SceneObjectProp());

	getScene()->add(prop, true);

	prop->create("resources/models/BlockGame.obj");

	prop->calculateAABB();

	std::shared_ptr<SceneObjectSurf> cartPole(new SceneObjectSurf());

	getRenderScene()->add(cartPole, false);

	cartPole->create();

	cartPole->_layer = 100.0f;

	std::shared_ptr<SceneObjectProp> sky(new SceneObjectProp());

	getScene()->add(sky, true);

	sky->create("resources/models/skybox1.obj");

	sky->_transform = pge::Matrix4x4f::scaleMatrix(pge::Vec3f(100.0f, 100.0f, 100.0f));

	sky->calculateAABB();

	std::shared_ptr<pge::SceneObjectDirectionalLightShadowed> directionalLight(new pge::SceneObjectDirectionalLightShadowed());

	getScene()->add(directionalLight);

	directionalLight->create(pLighting, 3, 2048, 0.2f, 100.0f, 0.6f);

	directionalLight->setDirection(pge::Vec3f(-0.2523f, -0.9423f, -0.424f).normalized());

	directionalLight->setColor(pge::Vec3f(0.7f, 0.7f, 0.7f));

	// ------------------------------------------- Image Effects -------------------------------------------

	std::shared_ptr<pge::SceneObjectEffectBuffer> effectBuffer(new pge::SceneObjectEffectBuffer());

	getRenderScene()->addNamed(effectBuffer, "ebuf", false);

	effectBuffer->create(0.5f);

	std::shared_ptr<pge::Asset> assetBlurHorizontal;

	getScene()->getAssetManager("shader", pge::Shader::assetFactory)->getAsset("NONE resources/shaders/noTransformVertex.vert resources/shaders/blurHorizontal.frag", assetBlurHorizontal);

	std::shared_ptr<pge::Shader> blurShaderHorizontal = std::static_pointer_cast<pge::Shader>(assetBlurHorizontal);

	std::shared_ptr<pge::Asset> assetBlurVertical;

	getScene()->getAssetManager("shader", pge::Shader::assetFactory)->getAsset("NONE resources/shaders/noTransformVertex.vert resources/shaders/blurVertical.frag", assetBlurVertical);

	std::shared_ptr<pge::Shader> blurShaderVertical = std::static_pointer_cast<pge::Shader>(assetBlurVertical);

	std::shared_ptr<pge::Asset> assetRenderImage;

	getScene()->getAssetManager("shader", pge::Shader::assetFactory)->getAsset("NONE resources/shaders/noTransformVertex.vert resources/shaders/renderImage.frag", assetRenderImage);

	std::shared_ptr<pge::Shader> renderImageShader = std::static_pointer_cast<pge::Shader>(assetRenderImage);

	std::shared_ptr<pge::Asset> assetRenderMultImage;

	getScene()->getAssetManager("shader", pge::Shader::assetFactory)->getAsset("NONE resources/shaders/noTransformVertex.vert resources/shaders/renderImageMult.frag", assetRenderMultImage);

	std::shared_ptr<pge::Shader> renderImageMultShader = std::static_pointer_cast<pge::Shader>(assetRenderMultImage);

	std::shared_ptr<pge::Asset> assetNoise;

	getScene()->getAssetManager("tex2D", pge::Texture2D::assetFactory)->getAsset("resources/textures/noise.bmp", assetNoise);

	std::shared_ptr<pge::Texture2D> noiseMap = std::static_pointer_cast<pge::Texture2D>(assetNoise);

	noiseMap->bind();

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	std::shared_ptr<pge::Asset> assetBlurHorizontalEdgeAware;

	getScene()->getAssetManager("shader", pge::Shader::assetFactory)->getAsset("NONE resources/shaders/noTransformVertex.vert resources/shaders/blurHorizontalEdgeAware.frag", assetBlurHorizontalEdgeAware);

	std::shared_ptr<pge::Shader> blurShaderHorizontalEdgeAware = std::static_pointer_cast<pge::Shader>(assetBlurHorizontalEdgeAware);

	std::shared_ptr<pge::Asset> assetBlurVerticalEdgeAware;

	getScene()->getAssetManager("shader", pge::Shader::assetFactory)->getAsset("NONE resources/shaders/noTransformVertex.vert resources/shaders/blurVerticalEdgeAware.frag", assetBlurVerticalEdgeAware);

	std::shared_ptr<pge::Shader> blurShaderVerticalEdgeAware = std::static_pointer_cast<pge::Shader>(assetBlurVerticalEdgeAware);

	// SSAO

	/*std::shared_ptr<pge::Shader> ssaoShader(new pge::Shader());

	ssaoShader->createAsset("NONE resources/shaders/noTransformVertex.vert resources/shaders/imageeffects/ssao.frag");

	std::shared_ptr<pge::SceneObjectSSAO> ssao(new pge::SceneObjectSSAO());

	getRenderScene()->add(ssao, false);

	ssao->create(blurShaderHorizontal, blurShaderVertical, ssaoShader, renderImageShader, noiseMap);

	ssao->_ssaoRadius = 0.1f;
	ssao->_ssaoStrength = 1.0f;
	ssao->_blurRadius = 0.002f;
	ssao->_numBlurPasses = 1;*/

	// SSR

	std::shared_ptr<pge::TextureCube> cubeMap(new pge::TextureCube());

	cubeMap->createAsset(
		"resources/environmentmaps/skybox1/CloudyLightRaysLeft2048.png "
		"resources/environmentmaps/skybox1/CloudyLightRaysRight2048.png "
		"resources/environmentmaps/skybox1/CloudyLightRaysBack2048.png "
		"resources/environmentmaps/skybox1/CloudyLightRaysFront2048.png "
		"resources/environmentmaps/skybox1/CloudyLightRaysDown2048.png "
		"resources/environmentmaps/skybox1/CloudyLightRaysUp2048.png "
		);

	std::shared_ptr<pge::Shader> ssrShader(new pge::Shader());

	ssrShader->createAsset("NONE resources/shaders/noTransformVertex.vert resources/shaders/imageeffects/ssr.frag");

	std::shared_ptr<pge::SceneObjectSSR> ssr(new pge::SceneObjectSSR());

	getRenderScene()->add(ssr, false);

	ssr->create(blurShaderHorizontalEdgeAware, blurShaderVerticalEdgeAware, ssrShader, renderImageShader, cubeMap, noiseMap);

	ssr->_layer = 1.0f;

	// Light Scattering

	/*std::shared_ptr<pge::Shader> lightScatteringShader(new pge::Shader());

	lightScatteringShader->createAsset("NONE resources/shaders/noTransformVertex.vert resources/shaders/imageeffects/lightScattering.frag");

	std::shared_ptr<pge::SceneObjectLightScattering> lightScattering(new pge::SceneObjectLightScattering());

	getRenderScene()->add(lightScattering, false);

	lightScattering->create(blurShaderHorizontal, blurShaderVertical, lightScatteringShader, renderImageShader);

	lightScattering->_layer = 1.5f;

	lightScattering->_lightSourcePosition = -light->getDirection() * 200.0f;
	lightScattering->_lightSourceColor = pge::Vec3f(1.0f, 0.9f, 0.8f) * 0.5f;*/

	// Depth of field

	/*std::shared_ptr<pge::Shader> depthOfFieldBlurShaderHorizontal(new pge::Shader());

	depthOfFieldBlurShaderHorizontal->createAsset("NONE resources/shaders/noTransformVertex.vert resources/shaders/imageeffects/depthOfFieldBlurHorizontal.frag");

	std::shared_ptr<pge::Shader> depthOfFieldBlurShaderVertical(new pge::Shader());

	depthOfFieldBlurShaderVertical->createAsset("NONE resources/shaders/noTransformVertex.vert resources/shaders/imageeffects/depthOfFieldBlurVertical.frag");

	std::shared_ptr<pge::SceneObjectDepthOfField> depthOfField(new pge::SceneObjectDepthOfField());

	getRenderScene()->add(depthOfField, false);

	depthOfField->create(depthOfFieldBlurShaderHorizontal, depthOfFieldBlurShaderVertical, renderImageShader);

	depthOfField->_layer = 1.5f;

	depthOfField->_focalDistance = 9.0f;
	depthOfField->_focalRange = 0.4f;
	depthOfField->_blurRadius = 0.002f;
	depthOfField->_numBlurPasses = 1;*/

	// FXAA

	std::shared_ptr<pge::Shader> lumaShader(new pge::Shader());

	lumaShader->createAsset("NONE resources/shaders/noTransformVertex.vert resources/shaders/imageeffects/lumaRender.frag");

	std::shared_ptr<pge::Shader> fxaaShader(new pge::Shader());

	fxaaShader->createAsset("NONE resources/shaders/noTransformVertex.vert resources/shaders/imageeffects/fxaa.frag");

	std::shared_ptr<pge::SceneObjectFXAA> fxaa(new pge::SceneObjectFXAA());

	getRenderScene()->add(fxaa, false);

	fxaa->create(fxaaShader, lumaShader);

	destroy();
}