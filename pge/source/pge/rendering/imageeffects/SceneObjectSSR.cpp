#include "SceneObjectSSR.h"

using namespace pge;

void SceneObjectSSR::create(const std::shared_ptr<Shader> &blurShaderHorizontalEdgeAware,
    const std::shared_ptr<Shader> &blurShaderVerticalEdgeAware,
    const std::shared_ptr<Shader> &ssrShader,
    const std::shared_ptr<Shader> &renderImageShader,
    const std::shared_ptr<TextureCube> &cubeMap,
    const std::shared_ptr<Texture2D> &noiseMap)
{
    renderMask = 0xffff;

    this->blurShaderHorizontalEdgeAware = blurShaderHorizontalEdgeAware;
    this->blurShaderVerticalEdgeAware = blurShaderVerticalEdgeAware;
    this->ssrShader = ssrShader;
    this->renderImageShader = renderImageShader;
    this->cubeMap = cubeMap;
    this->noiseMap = noiseMap;

    ssrShader->bind();

    ssrShader->setShaderTexture("pgeGBufferColor", getRenderScene()->gBuffer.getTextureID(GBuffer::diffuseAndSpecular), GL_TEXTURE_2D);
    ssrShader->setShaderTexture("pgeGBufferPosition", getRenderScene()->gBuffer.getTextureID(GBuffer::positionAndEmissive), GL_TEXTURE_2D);
    ssrShader->setShaderTexture("pgeGBufferNormal", getRenderScene()->gBuffer.getTextureID(GBuffer::normalAndShininess), GL_TEXTURE_2D);
    ssrShader->setShaderTexture("pgeGBufferEffect", getRenderScene()->gBuffer.getEffectTextureID(), GL_TEXTURE_2D);
    ssrShader->setShaderTexture("pgeCubeMap", cubeMap->getTextureID(), GL_TEXTURE_CUBE_MAP);
    ssrShader->setShaderTexture("pgeNoiseMap", noiseMap->getTextureID(), GL_TEXTURE_2D);

    blurShaderHorizontalEdgeAware->bind();

    blurShaderHorizontalEdgeAware->setShaderTexture("pgeGBufferPosition", getRenderScene()->gBuffer.getTextureID(GBuffer::positionAndEmissive), GL_TEXTURE_2D);
    blurShaderHorizontalEdgeAware->setShaderTexture("pgeGBufferNormal", getRenderScene()->gBuffer.getTextureID(GBuffer::normalAndShininess), GL_TEXTURE_2D);

    blurShaderVerticalEdgeAware->bind();

    blurShaderVerticalEdgeAware->setShaderTexture("pgeGBufferPosition", getRenderScene()->gBuffer.getTextureID(GBuffer::positionAndEmissive), GL_TEXTURE_2D);
    blurShaderVerticalEdgeAware->setShaderTexture("pgeGBufferNormal", getRenderScene()->gBuffer.getTextureID(GBuffer::normalAndShininess), GL_TEXTURE_2D);
}

void SceneObjectSSR::onAdd() {
    effectBuffer = getScene()->getNamed("ebuf");

    assert(effectBuffer.isAlive());
}

void SceneObjectSSR::postRender() {
    SceneObjectEffectBuffer* pEffectBuffer = static_cast<SceneObjectEffectBuffer*>(effectBuffer.get());

    Vec2f sizeInv(1.0f / static_cast<float>(pEffectBuffer->fullPing->getWidth()), 1.0f / static_cast<float>(pEffectBuffer->fullPing->getHeight()));

    glDisable(GL_DEPTH_TEST);

    ssrShader->bind();

    Matrix3x3f normalMatrixInv;
    //Matrix4x4f projectionMatrixInv;

    getRenderScene()->logicCamera.getNormalMatrix().inverse(normalMatrixInv);
    //getRenderScene()->logicCamera.projectionMatrix.inverse(projectionMatrixInv);

    ssrShader->setUniformmat4("pgeProjectionMatrix", getRenderScene()->logicCamera.projectionMatrix);
    //_ssrShader->setUniformmat4("pgeProjectionMatrixInv", projectionMatrixInv);
    ssrShader->setUniformmat3("pgeNormalMatrixInv", normalMatrixInv);
    ssrShader->setUniformv2f("pgeSizeInv", sizeInv);

    ssrShader->bindShaderTextures();

    pEffectBuffer->fullPing->bind();
    pEffectBuffer->fullPing->setViewport();

    getRenderScene()->renderNormalizedQuad();

    for (size_t i = 0; i < numBlurPasses; i++) {
        blurShaderHorizontalEdgeAware->bind();

        blurShaderHorizontalEdgeAware->setShaderTexture("pgeScene", pEffectBuffer->fullPing->getTextureID(), GL_TEXTURE_2D);
        blurShaderHorizontalEdgeAware->setUniformf("pgeBlurSize", blurRadius);
        blurShaderHorizontalEdgeAware->setUniformv2f("pgeSizeInv", sizeInv);

        blurShaderHorizontalEdgeAware->bindShaderTextures();

        pEffectBuffer->fullPong->bind();
        pEffectBuffer->fullPong->setViewport();

        getRenderScene()->renderNormalizedQuad();

        blurShaderVerticalEdgeAware->bind();

        blurShaderVerticalEdgeAware->setShaderTexture("pgeScene", pEffectBuffer->fullPong->getTextureID(), GL_TEXTURE_2D);
        blurShaderVerticalEdgeAware->setUniformf("pgeBlurSize", blurRadius);
        blurShaderVerticalEdgeAware->setUniformv2f("pgeSizeInv", sizeInv);

        blurShaderVerticalEdgeAware->bindShaderTextures();

        pEffectBuffer->fullPing->bind();
        pEffectBuffer->fullPing->setViewport();

        getRenderScene()->renderNormalizedQuad();
    }

    Vec2f gBufferSizeInv(1.0f / static_cast<float>(getRenderScene()->gBuffer.getWidth()), 1.0f / static_cast<float>(getRenderScene()->gBuffer.getHeight()));

    // Blend with effect buffer
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    renderImageShader->bind();

    renderImageShader->setShaderTexture("pgeScene", pEffectBuffer->fullPing->getTextureID(), GL_TEXTURE_2D);

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