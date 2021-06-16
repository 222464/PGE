#include "SceneObjectEntryPoint.h"

#include "SceneObjectProp.h"
#include "SceneObjectPlayer.h"
#include "SceneObjectBox.h"
#include "SceneObjectQuadruped.h"

#include "../rendering/voxel/VoxelTerrain.h"
#include "../rendering/voxel/TerrainGenerator.h"
#include "../rendering/lighting/SceneObjectDirectionalLightShadowed.h"
#include "../rendering/lighting/SceneObjectPointLightShadowed.h"

#include "../rendering/imageeffects/SceneObjectEffectBuffer.h"
#include "../rendering/imageeffects/SceneObjectSSAO.h"
#include "../rendering/imageeffects/SceneObjectLightScattering.h"
#include "../rendering/imageeffects/SceneObjectDepthOfField.h"
#include "../rendering/imageeffects/SceneObjectSSR.h"
#include "../rendering/imageeffects/SceneObjectFXAA.h"
#include "../rendering/imageeffects/SceneObjectFog.h"

#include "SceneObjectFloatingCamera.h"
#include "SceneObjectOrbitCamera.h"

#include "../system/SoftwareImage2D.h"

void SceneObjectEntryPoint::onAdd() {
    // Physics

    std::shared_ptr<pge::SceneObjectPhysicsWorld> physicsWorld(new pge::SceneObjectPhysicsWorld());

    getRenderScene()->addNamed(physicsWorld, "physWrld");

    pge::SceneObjectRef lighting = getScene()->getNamed("lighting");

    pge::SceneObjectLighting* pLighting = static_cast<pge::SceneObjectLighting*>(lighting.get());

    pLighting->ambientLight = pge::Vec3f(0.01f, 0.01f, 0.01f);

    /*std::shared_ptr<pge::SceneObjectDirectionalLightShadowed> light(new pge::SceneObjectDirectionalLightShadowed());

    getRenderScene()->add(light);

    light->create(pLighting, 2, 2048, 0.5f, 100.0f, 0.6f);

    light->setDirection(pge::Vec3f(-0.4f, -1.0f, 0.6f).normalized());

    light->setColor(pge::Vec3f(1.0f, 1.0f, 1.0f));*/

    // GUI

    /*std::shared_ptr<SceneObjectGUI> gui(new SceneObjectGUI());

    getRenderScene()->addNamed(gui, "gui", false);

    gui->layer = 2.0f;*/

    // Control

    std::shared_ptr<SceneObjectFloatingCamera> camera(new SceneObjectFloatingCamera());

    getRenderScene()->addNamed(camera, "flycam", false);

    // Terrain
    std::shared_ptr<pge::VoxelTerrain> terrain(new pge::VoxelTerrain());

    getScene()->add(terrain, false);

    std::shared_ptr<pge::Asset> assetTerrainGBufferRenderShader;

    getScene()->getAssetManager("shader", pge::Shader::assetFactory)->getAsset("NONE resources/shaders/voxel/gBufferRenderBumpVoxel.vert resources/shaders/voxel/gBufferRenderBumpVoxel.frag", assetTerrainGBufferRenderShader);

    std::shared_ptr<pge::Shader> terrainGBufferRenderShader = std::static_pointer_cast<pge::Shader>(assetTerrainGBufferRenderShader);

    std::shared_ptr<pge::Asset> assetTerrainGBufferRenderDepthShader;

    getScene()->getAssetManager("shader", pge::Shader::assetFactory)->getAsset("NONE resources/shaders/voxel/gBufferRenderVoxelDepth.vert resources/shaders/voxel/gBufferRenderVoxelDepth.frag", assetTerrainGBufferRenderDepthShader);

    std::shared_ptr<pge::Shader> terrainGBufferRenderDepthShader = std::static_pointer_cast<pge::Shader>(assetTerrainGBufferRenderDepthShader);

    std::shared_ptr<pge::Texture2DArray> diffuseArray(new pge::Texture2DArray());

    diffuseArray->createAsset(
        "resources/textures/sand.png "
        "resources/textures/sand.png "
        "resources/textures/sand.png "
    );

    diffuseArray->bind();
    diffuseArray->genMipMaps();

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

    std::shared_ptr<pge::Texture2DArray> normalArray(new pge::Texture2DArray());

    normalArray->createAsset(
        "resources/textures/sand_normal.png "
        "resources/textures/sand_normal.png "
        "resources/textures/sand_normal.png "
    );

    normalArray->bind();
    normalArray->genMipMaps();

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

    terrain->create(pge::Point3i(2, 2, 2), terrainGBufferRenderShader, terrainGBufferRenderDepthShader, diffuseArray, normalArray);

    terrain->generate(&pge::terrainGenerator0, 1234);

    // Map

    /*std::shared_ptr<pge::Map3DWS> map(new pge::Map3DWS());

    map->settings.pScene = getScene();

    map->createAsset("resources/maps/horrorMap.3dw");

    pge::addMapLights(*map, getScene());*/

    //std::shared_ptr<SceneObjectProp> prop(new SceneObjectProp());

    //getScene()->add(prop, false);

    //prop->create("resources/models/QuadrupedWorld.obj");

    //prop->calculateAABB();

    //std::shared_ptr<SceneObjectProp> prop2(new SceneObjectProp());

    //getScene()->add(prop2, false);

    //prop2->create("resources/models/vokselia_spawn.obj");
    ////prop2->create("resources/models/test.obj");

    //prop2->calculateAABB();

    //std::shared_ptr<SceneObjectProp> sky(new SceneObjectProp());

    //getScene()->add(sky, true);

    //sky->create("resources/models/skybox1.obj");

    //sky->transform = pge::Matrix4x4f::scaleMatrix(pge::Vec3f(100.0f, 100.0f, 100.0f));

    //sky->calculateAABB();

    std::shared_ptr<pge::SceneObjectDirectionalLightShadowed> directionalLight(new pge::SceneObjectDirectionalLightShadowed());

    getScene()->add(directionalLight, false);

    directionalLight->create(pLighting, 3, 2048, 1.0f, 100.0f, 2.0f);

    directionalLight->setDirection(pge::Vec3f(-0.4523f, -0.9423f, -0.424f).normalized());

    directionalLight->setColor(pge::Vec3f(0.05f, 0.05f, 0.05f));

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

    ssao->ssaoRadius = 0.1f;
    ssao->ssaoStrength = 1.0f;
    ssao->blurRadius = 0.002f;
    ssao->numBlurPasses = 1;*/

    // SSR

    //std::shared_ptr<pge::TextureCube> cubeMap(new pge::TextureCube());

    //cubeMap->createAsset(
    //    "resources/environmentmaps/skybox1/CloudyLightRaysLeft2048.png "
    //    "resources/environmentmaps/skybox1/CloudyLightRaysRight2048.png "
    //    "resources/environmentmaps/skybox1/CloudyLightRaysBack2048.png "
    //    "resources/environmentmaps/skybox1/CloudyLightRaysFront2048.png "
    //    "resources/environmentmaps/skybox1/CloudyLightRaysDown2048.png "
    //    "resources/environmentmaps/skybox1/CloudyLightRaysUp2048.png "
    //    );

    //std::shared_ptr<pge::Shader> ssrShader(new pge::Shader());

    //ssrShader->createAsset("NONE resources/shaders/noTransformVertex.vert resources/shaders/imageeffects/ssr.frag");

    //std::shared_ptr<pge::SceneObjectSSR> ssr(new pge::SceneObjectSSR());

    //getRenderScene()->add(ssr, false);

    //ssr->create(blurShaderHorizontalEdgeAware, blurShaderVerticalEdgeAware, ssrShader, renderImageShader, cubeMap, noiseMap);

    //ssr->layer = 1.0f;

    // Light Scattering

    //std::shared_ptr<pge::Shader> lightScatteringShader(new pge::Shader());

    //lightScatteringShader->createAsset("NONE resources/shaders/noTransformVertex.vert resources/shaders/imageeffects/lightScattering.frag");

    //std::shared_ptr<pge::SceneObjectLightScattering> lightScattering(new pge::SceneObjectLightScattering());

    //getRenderScene()->add(lightScattering, false);

    //lightScattering->create(blurShaderHorizontal, blurShaderVertical, lightScatteringShader, renderImageShader);

    //lightScattering->layer = 1.5f;

    //lightScattering->lightSourcePosition = -directionalLight->getDirection() * 200.0f;
    //lightScattering->lightSourceColor = pge::Vec3f(1.0f, 0.9f, 0.8f) * 0.5f;

    // Depth of field

    std::shared_ptr<pge::Shader> depthOfFieldBlurShaderHorizontal(new pge::Shader());

    depthOfFieldBlurShaderHorizontal->createAsset("NONE resources/shaders/noTransformVertex.vert resources/shaders/imageeffects/depthOfFieldBlurHorizontal.frag");

    std::shared_ptr<pge::Shader> depthOfFieldBlurShaderVertical(new pge::Shader());

    depthOfFieldBlurShaderVertical->createAsset("NONE resources/shaders/noTransformVertex.vert resources/shaders/imageeffects/depthOfFieldBlurVertical.frag");

    std::shared_ptr<pge::SceneObjectDepthOfField> depthOfField(new pge::SceneObjectDepthOfField());

    getRenderScene()->add(depthOfField, false);

    depthOfField->create(depthOfFieldBlurShaderHorizontal, depthOfFieldBlurShaderVertical, renderImageShader);

    depthOfField->layer = 1.5f;

    depthOfField->focalDistance = 1.0f;
    depthOfField->focalRange = 4.0f;
    depthOfField->blurRadius = 0.0025f;
    depthOfField->numBlurPasses = 1;

    std::shared_ptr<pge::Shader> fogShader(new pge::Shader());

    fogShader->createAsset("NONE resources/shaders/noTransformVertex.vert resources/shaders/imageeffects/fog.frag");

    std::shared_ptr<pge::SceneObjectFog> fog(new pge::SceneObjectFog());

    getRenderScene()->add(fog, false);

    fog->create(fogShader);

    fog->fogColor = pge::Vec3f(0.02f, 0.0f, 0.0f);

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
