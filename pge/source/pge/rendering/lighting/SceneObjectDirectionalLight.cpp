#include "SceneObjectDirectionalLight.h"

using namespace pge;

SceneObjectDirectionalLight::SceneObjectDirectionalLight()
: color(1.0f, 1.0f, 1.0f),
direction(1.0f, 0.0f, 0.0f),
needsUniformBufferUpdate(true),
enabled(true)
{
    renderMask = 0xffff;
}

void SceneObjectDirectionalLight::create(SceneObjectLighting* pLighting) {
    lighting = pLighting;

    uniformBuffer.reset(new VBO());
    uniformBuffer->create();

    pLighting->directionalLightLightUBOShaderInterface->setUpBuffer(*uniformBuffer);

    updateUniformBuffer();
}

void SceneObjectDirectionalLight::setColor(const Vec3f &color) {
    this->color = color;

    needsUniformBufferUpdate = true;
}

void SceneObjectDirectionalLight::setDirection(const Vec3f &direction) {
    this->direction = direction;

    needsUniformBufferUpdate = true;
}

void SceneObjectDirectionalLight::updateUniformBuffer() {
    uniformBuffer->bind(GL_UNIFORM_BUFFER);

    SceneObjectLighting* pLighting = static_cast<SceneObjectLighting*>(lighting.get());

    pLighting->directionalLightLightUBOShaderInterface->setUniformv3f("pgeDirectionalLightDirection", getRenderScene()->renderCamera.getNormalMatrix() * direction);

    if (needsUniformBufferUpdate) {
        pLighting->directionalLightLightUBOShaderInterface->setUniformv3f("pgeDirectionalLightColor", color);

        needsUniformBufferUpdate = false;
    }
}

void SceneObjectDirectionalLight::deferredRender() {
    if (enabled && !getRenderScene()->renderingShadows && getRenderScene()->shaderSwitchesEnabled)
        static_cast<SceneObjectLighting*>(lighting.get())->directionalLights.push_back(*this);
}