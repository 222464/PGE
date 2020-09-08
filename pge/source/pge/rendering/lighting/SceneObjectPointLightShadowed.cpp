#include "SceneObjectPointLightShadowed.h"

using namespace pge;

SceneObjectPointLightShadowed::SceneObjectPointLightShadowed()
: position(0.0f, 0.0f, 0.0f),
color(1.0f, 1.0f, 1.0f),
range(1.0f),
shadowMapZNear(0.01f),
needsUniformBufferUpdate(true),
enabled(true)
{
	renderMask = 0xffff;

	aabb.lowerBound = Vec3f(-1.0f, -1.0f, -1.0f);
	aabb.lowerBound = Vec3f(1.0f, 1.0f, 1.0f);

	aabb.calculateHalfDims();
	aabb.calculateCenter();

	updateFaces.fill(true);

	faceObjects.reset(new std::array<std::vector<AABB3D>, 6>());
}

void SceneObjectPointLightShadowed::create(SceneObjectLighting* pLighting, unsigned int size) {
	lighting = pLighting;

	uniformBuffer.reset(new VBO());
	uniformBuffer->create();

	pLighting->pointLightShadowedLightUBOShaderInterface->setUpBuffer(*uniformBuffer);

	cubeMap.reset(new CubeMapDepthRT());
	cubeMap->create(size, CubeMapDepthRT::_16);

	updateUniformBuffer();
}

void SceneObjectPointLightShadowed::setPosition(const Vec3f &position) {
	this->position = position;

	needsUniformBufferUpdate = true;

	aabb.setCenter(position);

	updateFaces.fill(true);

	updateAABB();
}

void SceneObjectPointLightShadowed::setColor(const Vec3f &color) {
	this->color = color;

	needsUniformBufferUpdate = true;
}

void SceneObjectPointLightShadowed::setRange(float range) {
	this->range = range;

	needsUniformBufferUpdate = true;

	aabb.setHalfDims(Vec3f(range, range, range));

	updateFaces.fill(true);

	updateAABB();
}

void SceneObjectPointLightShadowed::updateUniformBuffer() {
	uniformBuffer->bind(GL_UNIFORM_BUFFER);

	SceneObjectLighting* pLighting = static_cast<SceneObjectLighting*>(lighting.get());

	pLighting->pointLightShadowedLightUBOShaderInterface->setUniformv3f("pgePointLightPosition", getRenderScene()->renderCamera.getViewMatrix() * position);
	pLighting->pointLightShadowedLightUBOShaderInterface->setUniformmat4("pgeToLightSpace", Matrix4x4f::translateMatrix(-position) * getRenderScene()->renderCamera.getViewInverseMatrix());

	if (needsUniformBufferUpdate) {
		pLighting->pointLightShadowedLightUBOShaderInterface->setUniformv3f("pgePointLightColor", color);
		pLighting->pointLightShadowedLightUBOShaderInterface->setUniformf("pgePointLightRange", range);
		pLighting->pointLightShadowedLightUBOShaderInterface->setUniformf("pgePointLightRangeInv", 1.0f / range);

		float zNear = shadowMapZNear;
		float zFar = range;
		float nearMinusFar = zNear - zFar;
		float proj22 = (zNear + zFar) / nearMinusFar;
		float proj23 = (2.0f * zNear * zFar) / nearMinusFar;

		pLighting->pointLightShadowedLightUBOShaderInterface->setUniformf("pgeProj22", proj22);
		pLighting->pointLightShadowedLightUBOShaderInterface->setUniformf("pgeProj23", proj23);

		needsUniformBufferUpdate = false;
	}
}

void SceneObjectPointLightShadowed::preRender() {
	if (enabled) {
		// Check which faces need updates
		std::array<std::vector<AABB3D>, 6> newFaceObjects;

		bool needsUpdate = false;

		for (unsigned char i = 0; i < 6; i++) {
			Camera c = cubeMap->getCamera(i, position, shadowMapZNear, range);

			std::vector<SceneObjectRef> visible;

			getRenderScene()->findVisible(c, visible);

			newFaceObjects[i].resize(visible.size());

			for (size_t j = 0; j < visible.size(); j++)
				newFaceObjects[i][j] = visible[j]->getAABB();

			if (newFaceObjects[i].size() != (*faceObjects)[i].size()) {
				updateFaces[i] = true;

				needsUpdate = true;
			}
			else {
				for (size_t j = 0; j < visible.size(); j++) {
					// Check for equality with previous
					if (newFaceObjects[i][j] != (*faceObjects)[i][j]) {
						updateFaces[i] = true;

						needsUpdate = true;

						break;
					}
				}
			}
		}

		(*faceObjects) = newFaceObjects;

		// ---------------------------- Render to Shadow Map ----------------------------

		if (needsUpdate) {
			SceneObjectLighting* pLighting = static_cast<SceneObjectLighting*>(lighting.get());

			getRenderScene()->useShader(pLighting->depthRenderShader.get());

			getRenderScene()->shaderSwitchesEnabled = false;
			getRenderScene()->renderingShadows = true;

			cubeMap->bind();
			cubeMap->setViewport();

			glClearDepth(1.0f);

			for (size_t i = 0; i < 6; i++)
			if (updateFaces[i]) {
				cubeMap->renderFace(getRenderScene(), position, i, shadowMapZNear, range, range);

				updateFaces[i] = false;
			}

			Shader::unbind();
			CubeMapDepthRT::unbind();

			getRenderScene()->shaderSwitchesEnabled = true;
			getRenderScene()->renderingShadows = false;

			PGE_GL_ERROR_CHECK();
		}
	}
}

void SceneObjectPointLightShadowed::deferredRender() {
	if (enabled && !getRenderScene()->renderingShadows && getRenderScene()->shaderSwitchesEnabled)
		static_cast<SceneObjectLighting*>(lighting.get())->shadowedPointLights.push_back(*this);
}

void SceneObjectPointLightShadowed::setShadowMap(Shader* pShader) {
	pShader->setShaderTexture("pgeShadowMap", cubeMap->getDepthTextureID(), GL_TEXTURE_CUBE_MAP);
}