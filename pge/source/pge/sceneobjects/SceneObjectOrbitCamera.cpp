#include "SceneObjectOrbitCamera.h"

#include "../scene/RenderScene.h"

#include "../util/Math.h"

#include "../rendering/lighting/SceneObjectPointLight.h"

SceneObjectOrbitCamera::SceneObjectOrbitCamera()
	: focusPoint(pge::Vec3f(0.0f, 1.0f, 0.0f)), distance(12.0f), tilt(0.8f),
	angle(0.0f), angularVelocity(0.08f)
{}

void SceneObjectOrbitCamera::onAdd() {
	input = getScene()->getNamed("buffIn");
}

void SceneObjectOrbitCamera::update(float dt) {
	pge::SceneObjectBufferedInput* pBufferedInput = static_cast<pge::SceneObjectBufferedInput*>(input.get());

	angle += dt * angularVelocity;

	pge::Vec3f pos = focusPoint + pge::Vec3f(cosf(angle) * distance, sinf(tilt) * distance, sinf(angle) * distance);

	pge::Quaternion lookDir = pge::Quaternion::getFromMatrix(pge::Matrix4x4f::cameraDirectionMatrix((focusPoint - pos).normalized(), pge::Vec3f(0.0f, 1.0f, 0.0f)));

	getRenderScene()->logicCamera.position = pos;
	getRenderScene()->logicCamera.rotation = lookDir;
}