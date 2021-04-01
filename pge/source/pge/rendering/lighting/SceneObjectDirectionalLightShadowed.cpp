#include "SceneObjectDirectionalLightShadowed.h"

using namespace pge;

SceneObjectDirectionalLightShadowed::SceneObjectDirectionalLightShadowed()
: color(1.0f, 1.0f, 1.0f),
direction(1.0f, 0.0f, 0.0f),
needsUniformBufferUpdate(true),
enabled(true),
downwardsRangeExtension(32.0f),
upwardsRangeExtension(256.0f),
sidewaysRangeExtensionMultiplier(2.0f)
{
	renderMask = 0xffff;
}

void SceneObjectDirectionalLightShadowed::create(SceneObjectLighting* pLighting, int numCascades, unsigned int cascadeResolution, float zNear, float zFar, float lambda) {
	assert(numCascades <= 3); // At most 3 cascades

	lighting = pLighting;

	uniformBuffer.reset(new VBO());
	uniformBuffer->create();

	pLighting->directionalLightShadowedLightUBOShaderInterface->setUpBuffer(*uniformBuffer);

	// Create shadow cascades
	this->zNear = zNear;
	this->zFar = zFar;

	splitDistances.resize(numCascades);
	lightBiasViewProjections.resize(numCascades);
	cascades.resize(numCascades);

	for (int i = 0; i < numCascades; i++) {
		float cascadeRatio = static_cast<float>(i + 1) / static_cast<float>(numCascades);
		float zNext = std::pow(cascadeRatio, lambda) * (zFar - zNext) + zNear;

		splitDistances[i] = zNext;

		cascades[i].reset(new DepthRT());
		cascades[i]->create(cascadeResolution, cascadeResolution, DepthRT::_16);
	}

	updateUniformBuffer();
}

void SceneObjectDirectionalLightShadowed::setColor(const Vec3f &color) {
	this->color = color;

	needsUniformBufferUpdate = true;
}

void SceneObjectDirectionalLightShadowed::setDirection(const Vec3f &direction) {
	this->direction = direction;

	needsUniformBufferUpdate = true;
}

void SceneObjectDirectionalLightShadowed::updateUniformBuffer() {
	uniformBuffer->bind(GL_UNIFORM_BUFFER);

	SceneObjectLighting* pLighting = static_cast<SceneObjectLighting*>(lighting.get());

	pLighting->directionalLightShadowedLightUBOShaderInterface->setUniformv3f("pgeDirectionalLightDirection", getRenderScene()->renderCamera.getNormalMatrix() * direction);
	pLighting->directionalLightShadowedLightUBOShaderInterface->setUniform("pgeLightBiasViewProjections", sizeof(float) * 16 * lightBiasViewProjections.size(), &lightBiasViewProjections[0]);

	if (needsUniformBufferUpdate) {
		pLighting->directionalLightShadowedLightUBOShaderInterface->setUniformv3f("pgeDirectionalLightColor", color);
		pLighting->directionalLightShadowedLightUBOShaderInterface->setUniformi("pgeNumCascades", static_cast<GLint>(cascades.size()));

		pLighting->directionalLightShadowedLightUBOShaderInterface->setUniform("pgeSplitDistances", sizeof(float) * splitDistances.size(), &splitDistances[0]);

		needsUniformBufferUpdate = false;
	}

	PGE_GL_ERROR_CHECK();
}

void SceneObjectDirectionalLightShadowed::getFrustumCornerPoints(float zDistance, std::array<Vec3f, 4> &points) {
	float frustumSideLength = (getRenderScene()->renderCamera.frustum.getCorner(0) - getRenderScene()->renderCamera.frustum.getCorner(1)).magnitude();

	float lerpFactor = zDistance / frustumSideLength;

	points[0] = lerp(getRenderScene()->renderCamera.frustum.getCorner(0), getRenderScene()->renderCamera.frustum.getCorner(1), lerpFactor);
	points[1] = lerp(getRenderScene()->renderCamera.frustum.getCorner(2), getRenderScene()->renderCamera.frustum.getCorner(3), lerpFactor);
	points[2] = lerp(getRenderScene()->renderCamera.frustum.getCorner(4), getRenderScene()->renderCamera.frustum.getCorner(5), lerpFactor);
	points[3] = lerp(getRenderScene()->renderCamera.frustum.getCorner(6), getRenderScene()->renderCamera.frustum.getCorner(7), lerpFactor);
}

void SceneObjectDirectionalLightShadowed::preRender() {
	if (enabled) {
		// ---------------------------- Update Projection ----------------------------

		std::vector<Camera> cascadeCameras(cascades.size());

		Vec3f viewForward(getRenderScene()->renderCamera.rotation * Vec3f(1.0f, 0.0f, 0.0f));
		Vec3f viewUp(getRenderScene()->renderCamera.rotation * Vec3f(0.0f, 1.0f, 0.0f));

		// Get frustum corner points
		std::array<Vec3f, 4> previousPoints;

		// Initial one
		getFrustumCornerPoints(zNear, previousPoints);

		for (size_t i = 0; i < cascades.size(); i++) {
			float dist = splitDistances[i];
	
			std::array<Vec3f, 4> points;

			getFrustumCornerPoints(dist, points);

			// Get average position for center of light camera
			Vec3f averagePosition(0.0f, 0.0f, 0.0f);

			for (size_t j = 0; j < 4; j++) {
				averagePosition += previousPoints[j];
				averagePosition += points[j];
			}

			averagePosition *= 0.125f;

			cascadeCameras[i].position = averagePosition;
			cascadeCameras[i].rotation = Quaternion::getFromMatrix(Matrix4x4f::cameraDirectionMatrix(direction, viewForward));

			cascadeCameras[i].updateViewMatrix();

			// Project and find bounds
			// First one is done manually
			AABB3D bounds;
			bounds.lowerBound = bounds.upperBound = cascadeCameras[i].getViewMatrix() * points[0];

			for (size_t j = 1; j < 4; j++) {
				// Expand AABB
				Vec3f projectedPoint(cascadeCameras[i].getViewMatrix() * points[j]);

				bounds.expand(projectedPoint);
			}

			// Go over previous points as well (TODO: DON'T RECALCULATE PROJECTION FOR PREVIOUS POINTS!)
			for (size_t j = 0; j < 4; j++) {
				Vec3f projectedPoint(cascadeCameras[i].getViewMatrix() * previousPoints[j]);

				// Expand AABB
				bounds.expand(projectedPoint);
			}

			bounds.calculateHalfDims();
			bounds.calculateCenter();

			// Use bounds to find cascade projection matrix
			cascadeCameras[i].projectionMatrix = Matrix4x4f::orthoMatrix(bounds.lowerBound.x * sidewaysRangeExtensionMultiplier, bounds.upperBound.x * sidewaysRangeExtensionMultiplier, // Projection
				bounds.lowerBound.y * sidewaysRangeExtensionMultiplier, bounds.upperBound.y * sidewaysRangeExtensionMultiplier,
				bounds.lowerBound.z - downwardsRangeExtension, bounds.upperBound.z + upwardsRangeExtension); // Extend range a bit (especially upwards) to include out of view occluders

			cascadeCameras[i].fullUpdate();

			previousPoints = points;
		}

		// ---------------------------- Render to cascades ----------------------------

		SceneObjectLighting* pLighting = static_cast<SceneObjectLighting*>(lighting.get());

		getRenderScene()->useShader(pLighting->depthRenderShader.get());

		getRenderScene()->shaderSwitchesEnabled = false;
		getRenderScene()->renderingShadows = true;

		Camera oldCamera = getRenderScene()->renderCamera;
	
		for (size_t i = 0; i < cascades.size(); i++) {
			// Bind the depth map FBO
			cascades[i]->bind();

			cascades[i]->setViewport();

			getRenderScene()->renderCamera = cascadeCameras[i];
			getRenderScene()->updateShaderUniforms();

			glClearDepth(1.0f);
			glClear(GL_DEPTH_BUFFER_BIT);

			getRenderScene()->setTransform(Matrix4x4f::identityMatrix());

			getRenderScene()->renderShadow();

			lightBiasViewProjections[i] = (getBiasMatrix() * cascadeCameras[i].getProjectionViewMatrix()) * oldCamera.getViewInverseMatrix();
		}

		Shader::unbind();
		DepthRT::unbind();

		getRenderScene()->renderCamera = oldCamera;
		getRenderScene()->updateShaderUniforms();

		getRenderScene()->shaderSwitchesEnabled = true;
		getRenderScene()->renderingShadows = false;

		PGE_GL_ERROR_CHECK();
	}
}

void SceneObjectDirectionalLightShadowed::deferredRender() {
	if (enabled && !getRenderScene()->renderingShadows && getRenderScene()->shaderSwitchesEnabled)
		static_cast<SceneObjectLighting*>(lighting.get())->shadowedDirectionalLights.push_back(*this);
}

void SceneObjectDirectionalLightShadowed::setCascadeShadowMaps(Shader* pShader) {
	for (int i = 0; i < cascades.size(); i++)
		pShader->setShaderTexture("pgeCascadeShadowMaps[" + std::to_string(i) + "]", cascades[i]->getDepthTextureID(), GL_TEXTURE_2D);

        // Set unused remainder to 0
        for (int i = cascades.size(); i < 3; i++)
		pShader->setShaderTexture("pgeCascadeShadowMaps[" + std::to_string(i) + "]", 0, GL_TEXTURE_2D);
}
