#include <pge/rendering/lighting/SceneObjectDirectionalLight.h>

using namespace pge;

SceneObjectDirectionalLight::SceneObjectDirectionalLight()
: _color(1.0f, 1.0f, 1.0f),
_direction(1.0f, 0.0f, 0.0f),
_needsUniformBufferUpdate(true),
_enabled(true)
{
	_renderMask = 0xffff;
}

void SceneObjectDirectionalLight::create(SceneObjectLighting* pLighting) {
	_lighting = pLighting;

	_uniformBuffer.reset(new VBO());
	_uniformBuffer->create();

	pLighting->_directionalLightLightUBOShaderInterface->setUpBuffer(*_uniformBuffer);

	updateUniformBuffer();
}

void SceneObjectDirectionalLight::setColor(const Vec3f &color) {
	_color = color;

	_needsUniformBufferUpdate = true;
}

void SceneObjectDirectionalLight::setDirection(const Vec3f &direction) {
	_direction = direction;

	_needsUniformBufferUpdate = true;
}

void SceneObjectDirectionalLight::updateUniformBuffer() {
	_uniformBuffer->bind(GL_UNIFORM_BUFFER);

	SceneObjectLighting* pLighting = static_cast<SceneObjectLighting*>(_lighting.get());

	pLighting->_directionalLightLightUBOShaderInterface->setUniformv3f("pgeDirectionalLightDirection", getRenderScene()->_renderCamera.getNormalMatrix() * _direction);

	if (_needsUniformBufferUpdate) {
		pLighting->_directionalLightLightUBOShaderInterface->setUniformv3f("pgeDirectionalLightColor", _color);

		_needsUniformBufferUpdate = false;
	}
}

void SceneObjectDirectionalLight::deferredRender() {
	if (_enabled && !getRenderScene()->_renderingShadows && getRenderScene()->_shaderSwitchesEnabled)
		static_cast<SceneObjectLighting*>(_lighting.get())->_directionalLights.push_back(*this);
}