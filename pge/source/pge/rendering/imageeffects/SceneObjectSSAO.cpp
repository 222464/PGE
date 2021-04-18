#include "SceneObjectSSAO.h"

using namespace pge;

void SceneObjectSSAO::create(const std::shared_ptr<Shader> &blurShaderHorizontal,
    const std::shared_ptr<Shader> &blurShaderVertical,
    const std::shared_ptr<Shader> &ssaoShader,
    const std::shared_ptr<Shader> &renderImageShader,
    const std::shared_ptr<Texture2D> &noiseMap)
{
    renderMask = 0xffff;

    this->blurShaderHorizontal = blurShaderHorizontal;
    this->blurShaderVertical = blurShaderVertical;
    this->ssaoShader = ssaoShader;
    this->renderImageShader = renderImageShader;
    this->noiseMap = noiseMap;

    ssaoShader->bind();

    ssaoShader->setShaderTexture("pgeGBufferPosition", getRenderScene()->gBuffer.getTextureID(GBuffer::positionAndEmissive), GL_TEXTURE_2D);
    ssaoShader->setShaderTexture("pgeGBufferNormal", getRenderScene()->gBuffer.getTextureID(GBuffer::normalAndShininess), GL_TEXTURE_2D);
    ssaoShader->setShaderTexture("pgeNoiseMap", noiseMap->getTextureID(), GL_TEXTURE_2D);
}

void SceneObjectSSAO::onAdd() {
    effectBuffer = getScene()->getNamed("ebuf");

    assert(effectBuffer.isAlive());
}

void SceneObjectSSAO::postRender() {
    SceneObjectEffectBuffer* pEffectBuffer = static_cast<SceneObjectEffectBuffer*>(effectBuffer.get());

    Vec2f sizeInv(1.0f / static_cast<float>(pEffectBuffer->ping->getWidth()), 1.0f / static_cast<float>(pEffectBuffer->ping->getHeight()));

    glDisable(GL_DEPTH_TEST);

    ssaoShader->bind();

    ssaoShader->setUniformf("pgeRadius", ssaoRadius);
    ssaoShader->setUniformf("pgeStrength", ssaoStrength);
    ssaoShader->setUniformmat4("pgeProjectionMatrix", getRenderScene()->logicCamera.projectionMatrix);
    ssaoShader->setUniformv2f("pgeSizeInv", sizeInv);

    ssaoShader->bindShaderTextures();

    pEffectBuffer->ping->bind();
    pEffectBuffer->ping->setViewport();

    getRenderScene()->renderNormalizedQuad();

    for (size_t i = 0; i < numBlurPasses; i++) {
        blurShaderHorizontal->bind();

        blurShaderHorizontal->setShaderTexture("pgeScene", pEffectBuffer->ping->getTextureID(), GL_TEXTURE_2D);
        blurShaderHorizontal->setUniformf("pgeBlurSize", blurRadius);
        blurShaderHorizontal->setUniformv2f("pgeSizeInv", sizeInv);

        blurShaderHorizontal->bindShaderTextures();

        pEffectBuffer->pong->bind();
        pEffectBuffer->pong->setViewport();

        getRenderScene()->renderNormalizedQuad();

        blurShaderVertical->bind();

        blurShaderVertical->setShaderTexture("pgeScene", pEffectBuffer->pong->getTextureID(), GL_TEXTURE_2D);
        blurShaderVertical->setUniformf("pgeBlurSize", blurRadius);
        blurShaderVertical->setUniformv2f("pgeSizeInv", sizeInv);

        blurShaderVertical->bindShaderTextures();

        pEffectBuffer->ping->bind();
        pEffectBuffer->ping->setViewport();

        getRenderScene()->renderNormalizedQuad();
    }

    Vec2f gBufferSizeInv(1.0f / static_cast<float>(getRenderScene()->gBuffer.getWidth()), 1.0f / static_cast<float>(getRenderScene()->gBuffer.getHeight()));

    // Blend with effect buffer
    glEnable(GL_BLEND);
    glBlendFunc(GL_DST_COLOR, GL_ZERO);

    renderImageShader->bind();

    renderImageShader->setShaderTexture("pgeScene", pEffectBuffer->ping->getTextureID(), GL_TEXTURE_2D);

    renderImageShader->setUniformv2f("pgeGBufferSizeInv", gBufferSizeInv);

    renderImageShader->bindShaderTextures();

    getRenderScene()->gBuffer.bind();
    getRenderScene()->gBuffer.setDrawEffect();
    getRenderScene()->gBuffer.setReadEffect();

    getRenderScene()->gBuffer.setViewport();

    getRenderScene()->renderNormalizedQuad();

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}