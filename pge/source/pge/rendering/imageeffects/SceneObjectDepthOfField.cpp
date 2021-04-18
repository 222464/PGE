#include "SceneObjectDepthOfField.h"

using namespace pge;

void SceneObjectDepthOfField::create(const std::shared_ptr<Shader> &depthOfFieldBlurShaderHorizontal,
    const std::shared_ptr<Shader> &depthOfFieldBlurShaderVertical,
    const std::shared_ptr<Shader> &renderImageShader)
{
    renderMask = 0xffff;

    this->depthOfFieldBlurShaderHorizontal = depthOfFieldBlurShaderHorizontal;
    this->depthOfFieldBlurShaderVertical = depthOfFieldBlurShaderVertical;
    this->renderImageShader = renderImageShader;
}

void SceneObjectDepthOfField::onAdd() {
    effectBuffer = getScene()->getNamed("ebuf");

    assert(effectBuffer.isAlive());
}

void SceneObjectDepthOfField::postRender() {
    SceneObjectEffectBuffer* pEffectBuffer = static_cast<SceneObjectEffectBuffer*>(effectBuffer.get());

    Vec2f gBufferSizeInv(1.0f / static_cast<float>(getRenderScene()->gBuffer.getWidth()), 1.0f / static_cast<float>(getRenderScene()->gBuffer.getHeight()));

    glDisable(GL_DEPTH_TEST);

    renderImageShader->bind();

    renderImageShader->setShaderTexture("pgeScene", getRenderScene()->gBuffer.getEffectTextureID(), GL_TEXTURE_2D);

    renderImageShader->setUniformv2f("pgeGBufferSizeInv", gBufferSizeInv);

    renderImageShader->bindShaderTextures();

    pEffectBuffer->fullPing->bind();
    pEffectBuffer->fullPing->setViewport();

    getRenderScene()->renderNormalizedQuad();

    for (size_t i = 0; i < numBlurPasses; i++) {
        depthOfFieldBlurShaderHorizontal->bind();

        depthOfFieldBlurShaderHorizontal->setShaderTexture("pgeScene", pEffectBuffer->fullPing->getTextureID(), GL_TEXTURE_2D);
        depthOfFieldBlurShaderHorizontal->setShaderTexture("pgeGBufferPosition", getRenderScene()->gBuffer.getTextureID(GBuffer::positionAndEmissive), GL_TEXTURE_2D);
        depthOfFieldBlurShaderHorizontal->setUniformf("pgeBlurSize", blurRadius);
        depthOfFieldBlurShaderHorizontal->setUniformf("pgeFocalDistance", focalDistance);
        depthOfFieldBlurShaderHorizontal->setUniformf("pgeFocalRange", focalRange);
        depthOfFieldBlurShaderHorizontal->setUniformv2f("pgeSizeInv", gBufferSizeInv);

        depthOfFieldBlurShaderHorizontal->bindShaderTextures();

        pEffectBuffer->fullPong->bind();
        pEffectBuffer->fullPong->setViewport();

        getRenderScene()->renderNormalizedQuad();

        depthOfFieldBlurShaderVertical->bind();

        depthOfFieldBlurShaderVertical->setShaderTexture("pgeScene", pEffectBuffer->fullPong->getTextureID(), GL_TEXTURE_2D);
        depthOfFieldBlurShaderVertical->setShaderTexture("pgeGBufferPosition", getRenderScene()->gBuffer.getTextureID(GBuffer::positionAndEmissive), GL_TEXTURE_2D);
        depthOfFieldBlurShaderVertical->setUniformf("pgeBlurSize", blurRadius);
        depthOfFieldBlurShaderVertical->setUniformf("pgeFocalDistance", focalDistance);
        depthOfFieldBlurShaderVertical->setUniformf("pgeFocalRange", focalRange);
        depthOfFieldBlurShaderVertical->setUniformv2f("pgeSizeInv", gBufferSizeInv);

        depthOfFieldBlurShaderVertical->bindShaderTextures();

        pEffectBuffer->fullPing->bind();
        pEffectBuffer->fullPing->setViewport();

        getRenderScene()->renderNormalizedQuad();
    }

    renderImageShader->bind();

    renderImageShader->setShaderTexture("pgeScene", pEffectBuffer->fullPing->getTextureID(), GL_TEXTURE_2D);

    renderImageShader->setUniformv2f("pgeGBufferSizeInv", gBufferSizeInv);

    renderImageShader->bindShaderTextures();

    getRenderScene()->gBuffer.bind();
    getRenderScene()->gBuffer.setDrawEffect();
    getRenderScene()->gBuffer.setReadEffect();

    getRenderScene()->gBuffer.setViewport();

    getRenderScene()->renderNormalizedQuad();

    glEnable(GL_DEPTH_TEST);
}