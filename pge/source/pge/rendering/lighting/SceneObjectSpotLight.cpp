#include "SceneObjectSpotLight.h"

using namespace pge;

SceneObjectSpotLight::SceneObjectSpotLight()
: position(0.0f, 0.0f, 0.0f),
color(1.0f, 1.0f, 1.0f),
range(1.0f),
direction(1.0f, 0.0f, 0.0f),
spotAngle(piOver4),
exponent(8.0f),
needsUniformBufferUpdate(true),
enabled(true)
{
	renderMask = 0xffff;

	updateSpotLightInternals();
}

void SceneObjectSpotLight::create(SceneObjectLighting* pLighting) {
	lighting = pLighting;

	uniformBuffer.reset(new VBO());
	uniformBuffer->create();

	pLighting->spotLightLightUBOShaderInterface->setUpBuffer(*uniformBuffer);

	updateUniformBuffer();
}

void SceneObjectSpotLight::setPosition(const Vec3f &position) {
	this->position = position;

	updateSpotLightInternals();

	needsUniformBufferUpdate = true;
}

void SceneObjectSpotLight::setColor(const Vec3f &color) {
	this->color = color;

	needsUniformBufferUpdate = true;
}

void SceneObjectSpotLight::setRange(float range) {
	this->range = range;

	updateSpotLightInternals();

	needsUniformBufferUpdate = true;
}

void SceneObjectSpotLight::setDirection(const Vec3f &direction) {
	this->direction = direction;

	updateSpotLightInternals();

	needsUniformBufferUpdate = true;
}

void SceneObjectSpotLight::setSpotAngle(float angle) {
	spotAngle = angle;

	updateSpotLightInternals();

	needsUniformBufferUpdate = true;
}

void SceneObjectSpotLight::setExponent(float exponent) {
	this->exponent = exponent;

	needsUniformBufferUpdate = true;
}

void SceneObjectSpotLight::updateUniformBuffer() {
	uniformBuffer->bind(GL_UNIFORM_BUFFER);

	SceneObjectLighting* pLighting = static_cast<SceneObjectLighting*>(&(*lighting));

	pLighting->spotLightLightUBOShaderInterface->setUniformv3f("pgeSpotLightColor", color);
	pLighting->spotLightLightUBOShaderInterface->setUniformf("pgeSpotLightRange", range);
	pLighting->spotLightLightUBOShaderInterface->setUniformf("pgeSpotLightRangeInv", 1.0f / range);
	pLighting->spotLightLightUBOShaderInterface->setUniformf("pgeSpotLightSpreadAngleCos", spotAngleCos);
	pLighting->spotLightLightUBOShaderInterface->setUniformf("pgeSpotLightSpreadAngleCosFlipInv", 1.0f / (1.0f - spotAngleCos));
	pLighting->spotLightLightUBOShaderInterface->setUniformf("pgeSpotLightExponent", exponent);

	if (needsUniformBufferUpdate) {
		pLighting->spotLightLightUBOShaderInterface->setUniformv3f("pgeSpotLightPosition", getRenderScene()->renderCamera.getViewMatrix() * position);
		pLighting->spotLightLightUBOShaderInterface->setUniformv3f("pgeSpotLightDirection", getRenderScene()->renderCamera.getNormalMatrix() * direction);

		needsUniformBufferUpdate = false;
	}
}

void SceneObjectSpotLight::deferredRender() {
	if (enabled && !getRenderScene()->renderingShadows && getRenderScene()->shaderSwitchesEnabled)
		static_cast<SceneObjectLighting*>(lighting.get())->spotLights.push_back(*this);
}

void SceneObjectSpotLight::updateSpotLightInternals() {
	spotAngleCos = cosf(spotAngle);
	endConeRadius = range * tanf(spotAngle);

	// X axis shifted unit AABB for transform
	aabb.setHalfDims(Vec3f(0.5f, 1.0f, 1.0f));
	aabb.setCenter(Vec3f(0.5f, 0.0f, 0.0f));

	// Update transform
	transform = Matrix4x4f::translateMatrix(position) * Matrix4x4f::directionMatrixAutoUp(direction) * Matrix4x4f::scaleMatrix(Vec3f(range, endConeRadius, endConeRadius));

	// Rotation
	aabb = aabb.getTransformedAABB(transform);

	// Tree update
	updateAABB();
}