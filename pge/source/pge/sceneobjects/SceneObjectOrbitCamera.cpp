#include <pge/sceneobjects/SceneObjectOrbitCamera.h>

#include <pge/scene/RenderScene.h>

#include <pge/util/Math.h>

#include <pge/rendering/lighting/SceneObjectPointLight.h>

SceneObjectOrbitCamera::SceneObjectOrbitCamera()
	: _focusPoint(pge::Vec3f(0.0f, 1.0f, 0.0f)), _distance(12.0f), _tilt(0.8f),
	_angle(0.0f), _angularVelocity(0.08f)
{}

void SceneObjectOrbitCamera::onAdd() {
	_input = getScene()->getNamed("buffIn");
}

void SceneObjectOrbitCamera::update(float dt) {
	pge::SceneObjectBufferedInput* pBufferedInput = static_cast<pge::SceneObjectBufferedInput*>(_input.get());

	_angle += dt * _angularVelocity;

	pge::Vec3f pos = _focusPoint + pge::Vec3f(std::cos(_angle) * _distance, std::sin(_tilt) * _distance, std::sin(_angle) * _distance);

	pge::Quaternion lookDir = pge::Quaternion::getFromMatrix(pge::Matrix4x4f::cameraDirectionMatrix((_focusPoint - pos).normalized(), pge::Vec3f(0.0f, 1.0f, 0.0f)));

	getRenderScene()->_logicCamera._position = pos;
	getRenderScene()->_logicCamera._rotation = lookDir;
}