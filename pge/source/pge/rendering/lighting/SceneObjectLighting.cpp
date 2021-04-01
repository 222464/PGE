#include "SceneObjectLighting.h"

#include "SceneObjectPointLight.h"
#include "SceneObjectPointLightShadowed.h"
#include "SceneObjectSpotLight.h"
#include "SceneObjectDirectionalLight.h"
#include "SceneObjectDirectionalLightShadowed.h"

using namespace pge;

void SceneObjectLighting::create(const std::shared_ptr<Shader> &ambientLightShader,
	const std::shared_ptr<Shader> &pointLightShader,
	const std::shared_ptr<Shader> &pointLightShadowedShader,
	const std::shared_ptr<Shader> &spotLightShader,
	const std::shared_ptr<Shader> &directionalLightShader,
	const std::shared_ptr<Shader> &directionalLightShadowedShader,
	const std::shared_ptr<Shader> &emissiveRenderShader,
	const std::shared_ptr<Shader> &depthRenderShader,
	const std::shared_ptr<StaticPositionModel> &sphereModel,
	const std::shared_ptr<StaticPositionModel> &coneModel,
	const std::shared_ptr<Texture2D> &noiseMap)
{
	this->ambientLightShader = ambientLightShader;
	this->pointLightShader = pointLightShader;
	this->pointLightShadowedShader = pointLightShadowedShader;
	this->spotLightShader = spotLightShader;
	this->directionalLightShader = directionalLightShader;
	this->directionalLightShadowedShader = directionalLightShadowedShader;
	this->emissiveRenderShader = emissiveRenderShader;
	this->depthRenderShader = depthRenderShader;
	this->noiseMap = noiseMap;

	this->sphereModel = sphereModel;
	this->coneModel = coneModel;

	Vec2f gBufferSizeInv(1.0f / static_cast<float>(getRenderScene()->gBuffer.getWidth()), 1.0f / static_cast<float>(getRenderScene()->gBuffer.getHeight()));

	ambientLightShader->bind();

	ambientLightShader->setShaderTexture("pgeScene", getRenderScene()->gBuffer.getTextureID(GBuffer::diffuseAndSpecular), GL_TEXTURE_2D);

	ambientLightShader->setUniformv2f("pgeGBufferSizeInv", gBufferSizeInv);

	pointLightShader->bind();

	pointLightShader->setShaderTexture("pgeGBufferPosition", getRenderScene()->gBuffer.getTextureID(GBuffer::positionAndEmissive), GL_TEXTURE_2D);
	pointLightShader->setShaderTexture("pgeGBufferNormal", getRenderScene()->gBuffer.getTextureID(GBuffer::normalAndShininess), GL_TEXTURE_2D);
	pointLightShader->setShaderTexture("pgeGBufferColor", getRenderScene()->gBuffer.getTextureID(GBuffer::diffuseAndSpecular), GL_TEXTURE_2D);

	pointLightShader->setUniformv2f("pgeGBufferSizeInv", gBufferSizeInv);

	const GLchar* pointLightNames[4] = {
		"pgePointLightPosition",
		"pgePointLightColor",
		"pgePointLightRange",
		"pgePointLightRangeInv"
	};

	pointLightLightUBOShaderInterface.reset(new UBOShaderInterface());
	pointLightLightUBOShaderInterface->create("pgePointLight", pointLightShader.get(), pointLightNames, 4);

	pointLightShadowedShader->bind();

	pointLightShadowedShader->setShaderTexture("pgeGBufferPosition", getRenderScene()->gBuffer.getTextureID(GBuffer::positionAndEmissive), GL_TEXTURE_2D);
	pointLightShadowedShader->setShaderTexture("pgeGBufferNormal", getRenderScene()->gBuffer.getTextureID(GBuffer::normalAndShininess), GL_TEXTURE_2D);
	pointLightShadowedShader->setShaderTexture("pgeGBufferColor", getRenderScene()->gBuffer.getTextureID(GBuffer::diffuseAndSpecular), GL_TEXTURE_2D);

	pointLightShadowedShader->setShaderTexture("pgeNoiseMap", noiseMap->getTextureID(), GL_TEXTURE_2D);

	pointLightShadowedShader->setUniformv2f("pgeGBufferSizeInv", gBufferSizeInv);

	const GLchar* pointLightShadowedNames[7] = {
		"pgePointLightPosition",
		"pgePointLightColor",
		"pgePointLightRange",
		"pgePointLightRangeInv",
		"pgeToLightSpace",
		"pgeProj22",
		"pgeProj23"
	};

	pointLightShadowedLightUBOShaderInterface.reset(new UBOShaderInterface());
	pointLightShadowedLightUBOShaderInterface->create("pgePointLightShadowed", pointLightShadowedShader.get(), pointLightShadowedNames, 7);

	spotLightShader->bind();

	spotLightShader->setShaderTexture("pgeGBufferPosition", getRenderScene()->gBuffer.getTextureID(GBuffer::positionAndEmissive), GL_TEXTURE_2D);
	spotLightShader->setShaderTexture("pgeGBufferNormal", getRenderScene()->gBuffer.getTextureID(GBuffer::normalAndShininess), GL_TEXTURE_2D);
	spotLightShader->setShaderTexture("pgeGBufferColor", getRenderScene()->gBuffer.getTextureID(GBuffer::diffuseAndSpecular), GL_TEXTURE_2D);

	spotLightShader->setUniformv2f("pgeGBufferSizeInv", gBufferSizeInv);

	const GLchar* spotLightNames[8] = {
		"pgeSpotLightPosition",
		"pgeSpotLightColor",
		"pgeSpotLightRange",
		"pgeSpotLightRangeInv",
		"pgeSpotLightDirection",
		"pgeSpotLightSpreadAngleCos",
		"pgeSpotLightSpreadAngleCosFlipInv",
		"pgeSpotLightExponent"
	};

	spotLightLightUBOShaderInterface.reset(new UBOShaderInterface());
	spotLightLightUBOShaderInterface->create("pgeSpotLight", spotLightShader.get(), spotLightNames, 8);

	directionalLightShader->bind();

	directionalLightShader->setShaderTexture("pgeGBufferPosition", getRenderScene()->gBuffer.getTextureID(GBuffer::positionAndEmissive), GL_TEXTURE_2D);
	directionalLightShader->setShaderTexture("pgeGBufferNormal", getRenderScene()->gBuffer.getTextureID(GBuffer::normalAndShininess), GL_TEXTURE_2D);
	directionalLightShader->setShaderTexture("pgeGBufferColor", getRenderScene()->gBuffer.getTextureID(GBuffer::diffuseAndSpecular), GL_TEXTURE_2D);

	directionalLightShader->setUniformv2f("pgeGBufferSizeInv", gBufferSizeInv);

	const GLchar* directionalLightNames[2] = {
		"pgeDirectionalLightColor",
		"pgeDirectionalLightDirection"
	};

	directionalLightLightUBOShaderInterface.reset(new UBOShaderInterface());
	directionalLightLightUBOShaderInterface->create("pgeDirectionalLight", directionalLightShader.get(), directionalLightNames, 2);

	directionalLightShadowedShader->bind();

	directionalLightShadowedShader->setShaderTexture("pgeGBufferPosition", getRenderScene()->gBuffer.getTextureID(GBuffer::positionAndEmissive), GL_TEXTURE_2D);
	directionalLightShadowedShader->setShaderTexture("pgeGBufferNormal", getRenderScene()->gBuffer.getTextureID(GBuffer::normalAndShininess), GL_TEXTURE_2D);
	directionalLightShadowedShader->setShaderTexture("pgeGBufferColor", getRenderScene()->gBuffer.getTextureID(GBuffer::diffuseAndSpecular), GL_TEXTURE_2D);

	directionalLightShadowedShader->setShaderTexture("pgeNoiseMap", noiseMap->getTextureID(), GL_TEXTURE_2D);

	directionalLightShadowedShader->setUniformv2f("pgeGBufferSizeInv", gBufferSizeInv);

	const GLchar* directionalLightShadowedNames[5] = {
		"pgeDirectionalLightColor",
		"pgeDirectionalLightDirection",
		"pgeNumCascades",
		"pgeSplitDistances",
		"pgeLightBiasViewProjections"
	};

	directionalLightShadowedLightUBOShaderInterface.reset(new UBOShaderInterface());
	directionalLightShadowedLightUBOShaderInterface->create("pgeDirectionalLightShadowed", directionalLightShadowedShader.get(), directionalLightShadowedNames, 5);

	emissiveRenderShader->bind();

	emissiveRenderShader->setShaderTexture("pgeGBufferPosition", getRenderScene()->gBuffer.getTextureID(GBuffer::positionAndEmissive), GL_TEXTURE_2D);
	emissiveRenderShader->setShaderTexture("pgeGBufferColor", getRenderScene()->gBuffer.getTextureID(GBuffer::diffuseAndSpecular), GL_TEXTURE_2D);
	
	emissiveRenderShader->setUniformv2f("pgeGBufferSizeInv", gBufferSizeInv);

	depthRenderShader->bind();
}

void SceneObjectLighting::add(SceneObjectPointLight &light) {
	pointLights.push_back(light);
}

void SceneObjectLighting::add(SceneObjectSpotLight &light) {
	spotLights.push_back(light);
}

void SceneObjectLighting::add(SceneObjectDirectionalLight &light) {
	directionalLights.push_back(light);
}

void SceneObjectLighting::add(SceneObjectPointLightShadowed &light) {
	shadowedPointLights.push_back(light);
}

/*
void SceneObjectLighting::add(SceneObjectSpotLightShadowed &light) {

}*/

void SceneObjectLighting::add(SceneObjectDirectionalLightShadowed &light) {
	shadowedDirectionalLights.push_back(light);
}

void SceneObjectLighting::postRender() {
	getRenderScene()->shaderSwitchesEnabled = false;

	glDisable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	emissiveRenderShader->bind();
	emissiveRenderShader->bindShaderTextures();

	getRenderScene()->renderNormalizedQuad();

	// Render ambient light
	if (ambientLight != Vec3f(0.0f, 0.0f, 0.0f)) {
		ambientLightShader->bind();
		ambientLightShader->bindShaderTextures();

		ambientLightShader->setUniformv3f("pgeColor", ambientLight);

		getRenderScene()->renderNormalizedQuad();
	}

	// --------------------------------------- Render all directional lights ---------------------------------------

	directionalLightShader->bind();
	directionalLightShader->bindShaderTextures();

	if (needsAttenuationUpdate)
		directionalLightShader->setUniformv3f("pgeAttenuation", attenuation);

	for (std::list<SceneObjectRef>::iterator it = directionalLights.begin(); it != directionalLights.end(); it++) {
		SceneObjectDirectionalLight* pDirectionalLight = static_cast<SceneObjectDirectionalLight*>((*it).get());

		pDirectionalLight->updateUniformBuffer();

		directionalLightLightUBOShaderInterface->bindBufferToSetIndex(*pDirectionalLight->uniformBuffer);

		getRenderScene()->renderNormalizedQuad();

		PGE_GL_ERROR_CHECK();
	}

	directionalLightShadowedShader->bind();

	if (needsAttenuationUpdate)
		directionalLightShadowedShader->setUniformv3f("pgeAttenuation", attenuation);

	for (std::list<SceneObjectRef>::iterator it = shadowedDirectionalLights.begin(); it != shadowedDirectionalLights.end(); it++) {
		SceneObjectDirectionalLightShadowed* pDirectionalLightShadowed = static_cast<SceneObjectDirectionalLightShadowed*>((*it).get());

		pDirectionalLightShadowed->updateUniformBuffer();

		directionalLightShadowedLightUBOShaderInterface->bindBufferToSetIndex(*pDirectionalLightShadowed->uniformBuffer);

		pDirectionalLightShadowed->setCascadeShadowMaps(directionalLightShadowedShader.get());

		directionalLightShadowedShader->bindShaderTextures();

		getRenderScene()->renderNormalizedQuad();

		PGE_GL_ERROR_CHECK();
	}

	glEnable(GL_DEPTH_TEST);

	// ------------------------------------------- Render all point lights -------------------------------------------

	pointLightShader->bind();
	pointLightShader->bindShaderTextures();

	if (needsAttenuationUpdate)
		pointLightShader->setUniformv3f("pgeAttenuation", attenuation);

	sphereModel->meshes[0]->setAttributes();

	for (std::list<SceneObjectRef>::iterator it = pointLights.begin(); it != pointLights.end(); it++) {
		SceneObjectPointLight* pPointLight = static_cast<SceneObjectPointLight*>((*it).get());

		pPointLight->updateUniformBuffer();

		pointLightLightUBOShaderInterface->bindBufferToSetIndex(*pPointLight->uniformBuffer);

		pPointLight->setTransform(getRenderScene());

		// If camera is inside light, do not perform depth test (would cull it away improperly)
		if ((pPointLight->getPosition() - getRenderScene()->renderCamera.position).magnitude() < pPointLight->getRange() * pointLightRangeScalar) {
			glCullFace(GL_FRONT);

			glDepthFunc(GL_GREATER);

			sphereModel->meshes[0]->renderFromAttributes();

			glDepthFunc(GL_LESS);

			glCullFace(GL_BACK);
		}
		else
			sphereModel->render();
	}

	// ------------------------------------------- Render all shadowed point lights -------------------------------------------

	pointLightShadowedShader->bind();
	pointLightShadowedShader->bindShaderTextures();

	if (needsAttenuationUpdate)
		pointLightShadowedShader->setUniformv3f("pgeAttenuation", attenuation);

	sphereModel->meshes[0]->setAttributes();

	for (std::list<SceneObjectRef>::iterator it = shadowedPointLights.begin(); it != shadowedPointLights.end(); it++) {
		SceneObjectPointLightShadowed* pPointLightShadowed = static_cast<SceneObjectPointLightShadowed*>((*it).get());

		pPointLightShadowed->updateUniformBuffer();

		pointLightShadowedLightUBOShaderInterface->bindBufferToSetIndex(*pPointLightShadowed->uniformBuffer);

		pPointLightShadowed->setTransform(getRenderScene());

		pPointLightShadowed->setShadowMap(pointLightShadowedShader.get());

		pointLightShadowedShader->bindShaderTextures();

		// If camera is inside light, do not perform depth test (would cull it away improperly)
		if ((pPointLightShadowed->getPosition() - getRenderScene()->renderCamera.position).magnitude() < pPointLightShadowed->getRange() * pointLightRangeScalar) {
			glCullFace(GL_FRONT);

			glDepthFunc(GL_GREATER);

			sphereModel->meshes[0]->renderFromAttributes();

			glDepthFunc(GL_LESS);

			glCullFace(GL_BACK);
		}
		else
			sphereModel->render();
	}

	// ------------------------------------------- Render all spot lights -------------------------------------------

	spotLightShader->bind();
	spotLightShader->bindShaderTextures();

	if (needsAttenuationUpdate)
		spotLightShader->setUniformv3f("pgeAttenuation", attenuation);

	coneModel->meshes[0]->setAttributes();

	for (std::list<SceneObjectRef>::iterator it = spotLights.begin(); it != spotLights.end(); it++) {
		SceneObjectSpotLight* pSpotLight = static_cast<SceneObjectSpotLight*>((*it).get());

		pSpotLight->updateUniformBuffer();

		spotLightLightUBOShaderInterface->bindBufferToSetIndex(*pSpotLight->uniformBuffer);

		pSpotLight->setTransform(getRenderScene());

		// If camera is inside light, do not perform depth test (would cull it away improperly)
		if ((pSpotLight->getPosition() - getRenderScene()->renderCamera.position).magnitude() < pSpotLight->getRange() * spotLightRangeScalar) {
			glCullFace(GL_FRONT);

			glDepthFunc(GL_GREATER);

			coneModel->meshes[0]->renderFromAttributes();

			glDepthFunc(GL_LESS);

			glCullFace(GL_BACK);
		}
		else
			coneModel->render();
	}

	// Clear all lists so that they may be repopulated next frame
	pointLights.clear();
	spotLights.clear();
	directionalLights.clear();
	shadowedPointLights.clear();
	//_shadowedSpotLights.clear();
	shadowedDirectionalLights.clear();

	glDisable(GL_BLEND);

	needsAttenuationUpdate = false;

	getRenderScene()->shaderSwitchesEnabled = true;
}

void SceneObjectLighting::setAttenuation(const Vec3f &attenuation) {
	this->attenuation = attenuation;

	needsAttenuationUpdate = true;
}

Matrix4x4f pge::getBiasMatrix() {
	Matrix4x4f mat;

	mat.elements[0] = 0.5f; mat.elements[4] = 0.0f; mat.elements[8] = 0.0f; mat.elements[12] = 0.5f;
	mat.elements[1] = 0.0f; mat.elements[5] = 0.5f; mat.elements[9] = 0.0f; mat.elements[13] = 0.5f;
	mat.elements[2] = 0.0f; mat.elements[6] = 0.0f; mat.elements[10] = 0.5f; mat.elements[14] = 0.5f;
	mat.elements[3] = 0.0f; mat.elements[7] = 0.0f; mat.elements[11] = 0.0f; mat.elements[15] = 1.0f;

	return mat;
}
