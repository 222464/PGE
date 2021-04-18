#include "SceneObjectPointLight.h"

using namespace pge;

SceneObjectPointLight::SceneObjectPointLight()
: position(0.0f, 0.0f, 0.0f),
color(1.0f, 1.0f, 1.0f),
range(1.0f),
needsUniformBufferUpdate(true),
enabled(true)
{
    renderMask = 0xffff;

    aabb.lowerBound = Vec3f(-1.0f, -1.0f, -1.0f);
    aabb.lowerBound = Vec3f(1.0f, 1.0f, 1.0f);

    aabb.calculateHalfDims();
    aabb.calculateCenter();
}

void SceneObjectPointLight::create(SceneObjectLighting* pLighting) {
    lighting = pLighting;

    uniformBuffer.reset(new VBO());
    uniformBuffer->create();

    pLighting->pointLightLightUBOShaderInterface->setUpBuffer(*uniformBuffer);

    updateUniformBuffer();
}

void SceneObjectPointLight::setPosition(const Vec3f &position) {
    this->position = position;

    needsUniformBufferUpdate = true;

    aabb.setCenter(position);

    updateAABB();
}

void SceneObjectPointLight::setColor(const Vec3f &color) {
    this->color = color;

    needsUniformBufferUpdate = true;
}

void SceneObjectPointLight::setRange(float range) {
    this->range = range;

    needsUniformBufferUpdate = true;

    aabb.setHalfDims(Vec3f(range, range, range));

    updateAABB();
}

void SceneObjectPointLight::updateUniformBuffer() {
    uniformBuffer->bind(GL_UNIFORM_BUFFER);

    SceneObjectLighting* pLighting = static_cast<SceneObjectLighting*>(lighting.get());

    pLighting->pointLightLightUBOShaderInterface->setUniformv3f("pgePointLightPosition", getRenderScene()->renderCamera.getViewMatrix() * position);

    if (needsUniformBufferUpdate) {
        pLighting->pointLightLightUBOShaderInterface->setUniformv3f("pgePointLightColor", color);
        pLighting->pointLightLightUBOShaderInterface->setUniformf("pgePointLightRange", range);
        pLighting->pointLightLightUBOShaderInterface->setUniformf("pgePointLightRangeInv", 1.0f / range);

        needsUniformBufferUpdate = false;
    }
}

void SceneObjectPointLight::deferredRender() {
    if (enabled && !getRenderScene()->renderingShadows && getRenderScene()->shaderSwitchesEnabled)
        static_cast<SceneObjectLighting*>(lighting.get())->pointLights.push_back(*this);
}