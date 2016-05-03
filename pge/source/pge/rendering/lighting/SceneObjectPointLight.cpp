#include <pge/rendering/lighting/SceneObjectPointLight.h>

using namespace pge;

SceneObjectPointLight::SceneObjectPointLight()
: _position(0.0f, 0.0f, 0.0f),
_color(1.0f, 1.0f, 1.0f),
_range(1.0f),
_needsUniformBufferUpdate(true),
_enabled(true)
{
	_renderMask = 0xffff;

	_aabb._lowerBound = Vec3f(-1.0f, -1.0f, -1.0f);
	_aabb._lowerBound = Vec3f(1.0f, 1.0f, 1.0f);

	_aabb.calculateHalfDims();
	_aabb.calculateCenter();
}

void SceneObjectPointLight::create(SceneObjectLighting* pLighting) {
	_lighting = pLighting;

	_uniformBuffer.reset(new VBO());
	_uniformBuffer->create();

	pLighting->_pointLightLightUBOShaderInterface->setUpBuffer(*_uniformBuffer);

	updateUniformBuffer();
}

void SceneObjectPointLight::setPosition(const Vec3f &position) {
	_position = position;

	_needsUniformBufferUpdate = true;

	_aabb.setCenter(position);

	updateAABB();
}

void SceneObjectPointLight::setColor(const Vec3f &color) {
	_color = color;

	_needsUniformBufferUpdate = true;
}

void SceneObjectPointLight::setRange(float range) {
	_range = range;

	_needsUniformBufferUpdate = true;

	_aabb.setHalfDims(Vec3f(_range, _range, _range));

	updateAABB();
}

void SceneObjectPointLight::updateUniformBuffer() {
	_uniformBuffer->bind(GL_UNIFORM_BUFFER);

	SceneObjectLighting* pLighting = static_cast<SceneObjectLighting*>(_lighting.get());

	pLighting->_pointLightLightUBOShaderInterface->setUniformv3f("pgePointLightPosition", getRenderScene()->_renderCamera.getViewMatrix() * _position);

	if (_needsUniformBufferUpdate) {
		pLighting->_pointLightLightUBOShaderInterface->setUniformv3f("pgePointLightColor", _color);
		pLighting->_pointLightLightUBOShaderInterface->setUniformf("pgePointLightRange", _range);
		pLighting->_pointLightLightUBOShaderInterface->setUniformf("pgePointLightRangeInv", 1.0f / _range);

		_needsUniformBufferUpdate = false;
	}
}

void SceneObjectPointLight::deferredRender() {
	if (_enabled && !getRenderScene()->_renderingShadows && getRenderScene()->_shaderSwitchesEnabled)
		static_cast<SceneObjectLighting*>(_lighting.get())->_pointLights.push_back(*this);
}