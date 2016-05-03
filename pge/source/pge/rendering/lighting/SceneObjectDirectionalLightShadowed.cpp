#include <pge/rendering/lighting/SceneObjectDirectionalLightShadowed.h>

using namespace pge;

SceneObjectDirectionalLightShadowed::SceneObjectDirectionalLightShadowed()
: _color(1.0f, 1.0f, 1.0f),
_direction(1.0f, 0.0f, 0.0f),
_needsUniformBufferUpdate(true),
_enabled(true),
_downwardsRangeExtension(32.0f),
_upwardsRangeExtension(256.0f),
_sidewaysRangeExtensionMultiplier(2.0f)
{
	_renderMask = 0xffff;
}

void SceneObjectDirectionalLightShadowed::create(SceneObjectLighting* pLighting, int numCascades, unsigned int cascadeResolution, float zNear, float zFar, float lambda) {
	assert(numCascades <= 3); // At most 3 cascades

	_lighting = pLighting;

	_uniformBuffer.reset(new VBO());
	_uniformBuffer->create();

	pLighting->_directionalLightShadowedLightUBOShaderInterface->setUpBuffer(*_uniformBuffer);

	// Create shadow cascades
	_zNear = zNear;
	_zFar = zFar;

	_splitDistances.resize(numCascades);
	_lightBiasViewProjections.resize(numCascades);
	_cascades.resize(numCascades);

	for (int i = 0; i < numCascades; i++) {
		float cascadeRatio = static_cast<float>(i + 1) / static_cast<float>(numCascades);
		float zNext = lambda * _zNear * powf(_zFar / _zNear, cascadeRatio) + (1.0f - lambda) * (_zNear + cascadeRatio * (_zFar - _zNear));

		_splitDistances[i] = zNext;

		_cascades[i].reset(new DepthRT());
		_cascades[i]->create(cascadeResolution, cascadeResolution, DepthRT::_16);
	}

	updateUniformBuffer();
}

void SceneObjectDirectionalLightShadowed::setColor(const Vec3f &color) {
	_color = color;

	_needsUniformBufferUpdate = true;
}

void SceneObjectDirectionalLightShadowed::setDirection(const Vec3f &direction) {
	_direction = direction;

	_needsUniformBufferUpdate = true;
}

void SceneObjectDirectionalLightShadowed::updateUniformBuffer() {
	_uniformBuffer->bind(GL_UNIFORM_BUFFER);

	SceneObjectLighting* pLighting = static_cast<SceneObjectLighting*>(_lighting.get());

	pLighting->_directionalLightShadowedLightUBOShaderInterface->setUniformv3f("pgeDirectionalLightDirection", getRenderScene()->_renderCamera.getNormalMatrix() * _direction);
	pLighting->_directionalLightShadowedLightUBOShaderInterface->setUniform("pgeLightBiasViewProjections", sizeof(float) * 16 * _lightBiasViewProjections.size(), &_lightBiasViewProjections[0]);

	if (_needsUniformBufferUpdate) {
		pLighting->_directionalLightShadowedLightUBOShaderInterface->setUniformv3f("pgeDirectionalLightColor", _color);
		pLighting->_directionalLightShadowedLightUBOShaderInterface->setUniformi("pgeNumCascades", static_cast<GLshort>(_cascades.size()));

		std::vector<Vec4f> splitDistancesVec4fs(_splitDistances.size());

		for (size_t i = 0; i < splitDistancesVec4fs.size(); i++)
			splitDistancesVec4fs[i] = Vec4f(_splitDistances[i], _splitDistances[i], _splitDistances[i], _splitDistances[i]);

		pLighting->_directionalLightShadowedLightUBOShaderInterface->setUniform("pgeSplitDistances", sizeof(Vec4f) * splitDistancesVec4fs.size(), &splitDistancesVec4fs[0]);

		_needsUniformBufferUpdate = false;
	}

	D3D_GL_ERROR_CHECK();
}

void SceneObjectDirectionalLightShadowed::getFrustumCornerPoints(float zDistance, std::array<Vec3f, 4> &points) {
	float frustumSideLength = (getRenderScene()->_renderCamera._frustum.getCorner(0) - getRenderScene()->_renderCamera._frustum.getCorner(1)).magnitude();

	float lerpFactor = zDistance / frustumSideLength;

	points[0] = lerp(getRenderScene()->_renderCamera._frustum.getCorner(0), getRenderScene()->_renderCamera._frustum.getCorner(1), lerpFactor);
	points[1] = lerp(getRenderScene()->_renderCamera._frustum.getCorner(2), getRenderScene()->_renderCamera._frustum.getCorner(3), lerpFactor);
	points[2] = lerp(getRenderScene()->_renderCamera._frustum.getCorner(4), getRenderScene()->_renderCamera._frustum.getCorner(5), lerpFactor);
	points[3] = lerp(getRenderScene()->_renderCamera._frustum.getCorner(6), getRenderScene()->_renderCamera._frustum.getCorner(7), lerpFactor);
}

void SceneObjectDirectionalLightShadowed::preRender() {
	if (_enabled) {
		// ---------------------------- Update Projection ----------------------------

		std::vector<Camera> cascadeCameras(_cascades.size());

		Vec3f viewForward(getRenderScene()->_renderCamera._rotation * Vec3f(1.0f, 0.0f, 0.0f));
		Vec3f viewUp(getRenderScene()->_renderCamera._rotation * Vec3f(0.0f, 1.0f, 0.0f));

		// Get frustum corner points
		std::array<Vec3f, 4> previousPoints;

		// Initial one
		getFrustumCornerPoints(_zNear, previousPoints);

		for (size_t i = 0; i < _cascades.size(); i++) {
			float dist = _splitDistances[i];
	
			std::array<Vec3f, 4> points;

			getFrustumCornerPoints(dist, points);

			// Get average position for center of light camera
			Vec3f averagePosition(0.0f, 0.0f, 0.0f);

			for (size_t j = 0; j < 4; j++) {
				averagePosition += previousPoints[j];
				averagePosition += points[j];
			}

			averagePosition *= 0.125f;

			cascadeCameras[i]._position = averagePosition;
			cascadeCameras[i]._rotation = Quaternion::getFromMatrix(Matrix4x4f::cameraDirectionMatrix(_direction, viewForward));

			cascadeCameras[i].updateViewMatrix();

			// Project and find bounds
			// First one is done manually
			AABB3D bounds;
			bounds._lowerBound = bounds._upperBound = cascadeCameras[i].getViewMatrix() * points[0];

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
			cascadeCameras[i]._projectionMatrix = Matrix4x4f::orthoMatrix(bounds._lowerBound.x * _sidewaysRangeExtensionMultiplier, bounds._upperBound.x * _sidewaysRangeExtensionMultiplier, // Projection
				bounds._lowerBound.y * _sidewaysRangeExtensionMultiplier, bounds._upperBound.y * _sidewaysRangeExtensionMultiplier,
				bounds._lowerBound.z - _downwardsRangeExtension, bounds._upperBound.z + _upwardsRangeExtension); // Extend range a bit (especially upwards) to include out of view occluders

			cascadeCameras[i].fullUpdate();

			previousPoints = points;
		}

		// ---------------------------- Render to cascades ----------------------------

		SceneObjectLighting* pLighting = static_cast<SceneObjectLighting*>(_lighting.get());

		getRenderScene()->useShader(pLighting->_depthRenderShader.get());

		getRenderScene()->_shaderSwitchesEnabled = false;
		getRenderScene()->_renderingShadows = true;

		Camera oldCamera = getRenderScene()->_renderCamera;
	
		for (size_t i = 0; i < _cascades.size(); i++) {
			// Bind the depth map FBO
			_cascades[i]->bind();

			_cascades[i]->setViewport();

			getRenderScene()->_renderCamera = cascadeCameras[i];
			getRenderScene()->updateShaderUniforms();

			glClearDepth(1.0f);
			glClear(GL_DEPTH_BUFFER_BIT);

			getRenderScene()->setTransform(Matrix4x4f::identityMatrix());

			getRenderScene()->renderShadow();

			_lightBiasViewProjections[i] = (getBiasMatrix() * cascadeCameras[i].getProjectionViewMatrix()) * oldCamera.getViewInverseMatrix();
		}

		Shader::unbind();
		DepthRT::unbind();

		getRenderScene()->_renderCamera = oldCamera;
		getRenderScene()->updateShaderUniforms();

		getRenderScene()->_shaderSwitchesEnabled = true;
		getRenderScene()->_renderingShadows = false;

		D3D_GL_ERROR_CHECK();
	}
}

void SceneObjectDirectionalLightShadowed::deferredRender() {
	if (_enabled && !getRenderScene()->_renderingShadows && getRenderScene()->_shaderSwitchesEnabled)
		static_cast<SceneObjectLighting*>(_lighting.get())->_shadowedDirectionalLights.push_back(*this);
}

void SceneObjectDirectionalLightShadowed::setCascadeShadowMaps(Shader* pShader) {
	for (int i = 0; i < _cascades.size(); i++)
		pShader->setShaderTexture("pgeCascadeShadowMaps[" + std::to_string(i) + "]", _cascades[i]->getDepthTextureID(), GL_TEXTURE_2D);

	for (int i = _cascades.size(); i < 3; i++)
		pShader->setShaderTexture("pgeCascadeShadowMaps[" + std::to_string(i) + "]", 0, GL_TEXTURE_2D);
}