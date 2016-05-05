#include <pge/rendering/lighting/SceneObjectSpotLight.h>

using namespace pge;

SceneObjectSpotLight::SceneObjectSpotLight()
: _position(0.0f, 0.0f, 0.0f),
_color(1.0f, 1.0f, 1.0f),
_range(1.0f),
_direction(1.0f, 0.0f, 0.0f),
_spotAngle(_piOver4),
_exponent(8.0f),
_needsUniformBufferUpdate(true),
_enabled(true)
{
	_renderMask = 0xffff;

	updateSpotLightInternals();
}

void SceneObjectSpotLight::create(SceneObjectLighting* pLighting) {
	_lighting = pLighting;

	_uniformBuffer.reset(new VBO());
	_uniformBuffer->create();

	pLighting->_spotLightLightUBOShaderInterface->setUpBuffer(*_uniformBuffer);

	updateUniformBuffer();
}

void SceneObjectSpotLight::setPosition(const Vec3f &position) {
	_position = position;

	updateSpotLightInternals();

	_needsUniformBufferUpdate = true;
}

void SceneObjectSpotLight::setColor(const Vec3f &color) {
	_color = color;

	_needsUniformBufferUpdate = true;
}

void SceneObjectSpotLight::setRange(float range) {
	_range = range;

	updateSpotLightInternals();

	_needsUniformBufferUpdate = true;
}

void SceneObjectSpotLight::setDirection(const Vec3f &direction) {
	_direction = direction;

	updateSpotLightInternals();

	_needsUniformBufferUpdate = true;
}

void SceneObjectSpotLight::setSpotAngle(float angle) {
	_spotAngle = angle;

	updateSpotLightInternals();

	_needsUniformBufferUpdate = true;
}

void SceneObjectSpotLight::setExponent(float exponent) {
	_exponent = exponent;

	_needsUniformBufferUpdate = true;
}

void SceneObjectSpotLight::updateUniformBuffer() {
	_uniformBuffer->bind(GL_UNIFORM_BUFFER);

	SceneObjectLighting* pLighting = static_cast<SceneObjectLighting*>(&(*_lighting));

	pLighting->_spotLightLightUBOShaderInterface->setUniformv3f("pgeSpotLightColor", _color);
	pLighting->_spotLightLightUBOShaderInterface->setUniformf("pgeSpotLightRange", _range);
	pLighting->_spotLightLightUBOShaderInterface->setUniformf("pgeSpotLightRangeInv", 1.0f / _range);
	pLighting->_spotLightLightUBOShaderInterface->setUniformf("pgeSpotLightSpreadAngleCos", _spotAngleCos);
	pLighting->_spotLightLightUBOShaderInterface->setUniformf("pgeSpotLightSpreadAngleCosFlipInv", 1.0f / (1.0f - _spotAngleCos));
	pLighting->_spotLightLightUBOShaderInterface->setUniformf("pgeSpotLightExponent", _exponent);

	if (_needsUniformBufferUpdate) {
		pLighting->_spotLightLightUBOShaderInterface->setUniformv3f("pgeSpotLightPosition", getRenderScene()->_renderCamera.getViewMatrix() * _position);
		pLighting->_spotLightLightUBOShaderInterface->setUniformv3f("pgeSpotLightDirection", getRenderScene()->_renderCamera.getNormalMatrix() * _direction);

		_needsUniformBufferUpdate = false;
	}
}

void SceneObjectSpotLight::deferredRender() {
	if (_enabled && !getRenderScene()->_renderingShadows && getRenderScene()->_shaderSwitchesEnabled)
		static_cast<SceneObjectLighting*>(_lighting.get())->_spotLights.push_back(*this);
}

void SceneObjectSpotLight::updateSpotLightInternals() {
	_spotAngleCos = cosf(_spotAngle);
	_endConeRadius = _range * tanf(_spotAngle);

	// X axis shifted unit AABB for transform
	_aabb.setHalfDims(Vec3f(0.5f, 1.0f, 1.0f));
	_aabb.setCenter(Vec3f(0.5f, 0.0f, 0.0f));

	// Update transform
	_transform = Matrix4x4f::translateMatrix(_position) * Matrix4x4f::directionMatrixAutoUp(_direction) * Matrix4x4f::scaleMatrix(Vec3f(_range, _endConeRadius, _endConeRadius));

	// Rotation
	_aabb = _aabb.getTransformedAABB(_transform);

	// Tree update
	updateAABB();
}