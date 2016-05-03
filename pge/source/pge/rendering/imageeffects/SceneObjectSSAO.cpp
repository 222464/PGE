#include <pge/rendering/imageeffects/SceneObjectSSAO.h>

using namespace pge;

void SceneObjectSSAO::create(const std::shared_ptr<Shader> &blurShaderHorizontal,
	const std::shared_ptr<Shader> &blurShaderVertical,
	const std::shared_ptr<Shader> &ssaoShader,
	const std::shared_ptr<Shader> &renderImageShader,
	const std::shared_ptr<Texture2D> &noiseMap)
{
	_renderMask = 0xffff;

	_blurShaderHorizontal = blurShaderHorizontal;
	_blurShaderVertical = blurShaderVertical;
	_ssaoShader = ssaoShader;
	_renderImageShader = renderImageShader;
	_noiseMap = noiseMap;

	_ssaoShader->bind();

	_ssaoShader->setShaderTexture("pgeGBufferPosition", getRenderScene()->_gBuffer.getTextureID(GBuffer::_positionAndEmissive), GL_TEXTURE_2D);
	_ssaoShader->setShaderTexture("pgeGBufferNormal", getRenderScene()->_gBuffer.getTextureID(GBuffer::_normalAndShininess), GL_TEXTURE_2D);
	_ssaoShader->setShaderTexture("pgeNoiseMap", _noiseMap->getTextureID(), GL_TEXTURE_2D);
}

void SceneObjectSSAO::onAdd() {
	_effectBuffer = getScene()->getNamed("ebuf");

	assert(_effectBuffer.isAlive());
}

void SceneObjectSSAO::postRender() {
	SceneObjectEffectBuffer* pEffectBuffer = static_cast<SceneObjectEffectBuffer*>(_effectBuffer.get());

	Vec2f sizeInv(1.0f / static_cast<float>(pEffectBuffer->_ping->getWidth()), 1.0f / static_cast<float>(pEffectBuffer->_ping->getHeight()));

	glDisable(GL_DEPTH_TEST);

	_ssaoShader->bind();

	_ssaoShader->setUniformf("pgeRadius", _ssaoRadius);
	_ssaoShader->setUniformf("pgeStrength", _ssaoStrength);
	_ssaoShader->setUniformmat4("pgeProjectionMatrix", getRenderScene()->_logicCamera._projectionMatrix);
	_ssaoShader->setUniformv2f("pgeSizeInv", sizeInv);

	_ssaoShader->bindShaderTextures();

	pEffectBuffer->_ping->bind();
	pEffectBuffer->_ping->setViewport();

	getRenderScene()->renderNormalizedQuad();

	for (size_t i = 0; i < _numBlurPasses; i++) {
		_blurShaderHorizontal->bind();

		_blurShaderHorizontal->setShaderTexture("pgeScene", pEffectBuffer->_ping->getTextureID(), GL_TEXTURE_2D);
		_blurShaderHorizontal->setUniformf("pgeBlurSize", _blurRadius);
		_blurShaderHorizontal->setUniformv2f("pgeSizeInv", sizeInv);

		_blurShaderHorizontal->bindShaderTextures();

		pEffectBuffer->_pong->bind();
		pEffectBuffer->_pong->setViewport();

		getRenderScene()->renderNormalizedQuad();

		_blurShaderVertical->bind();

		_blurShaderVertical->setShaderTexture("pgeScene", pEffectBuffer->_pong->getTextureID(), GL_TEXTURE_2D);
		_blurShaderVertical->setUniformf("pgeBlurSize", _blurRadius);
		_blurShaderVertical->setUniformv2f("pgeSizeInv", sizeInv);

		_blurShaderVertical->bindShaderTextures();

		pEffectBuffer->_ping->bind();
		pEffectBuffer->_ping->setViewport();

		getRenderScene()->renderNormalizedQuad();
	}

	Vec2f gBufferSizeInv(1.0f / static_cast<float>(getRenderScene()->_gBuffer.getWidth()), 1.0f / static_cast<float>(getRenderScene()->_gBuffer.getHeight()));

	// Blend with effect buffer
	glEnable(GL_BLEND);
	glBlendFunc(GL_DST_COLOR, GL_ZERO);

	_renderImageShader->bind();

	_renderImageShader->setShaderTexture("pgeScene", pEffectBuffer->_ping->getTextureID(), GL_TEXTURE_2D);

	_renderImageShader->setUniformv2f("pgeGBufferSizeInv", gBufferSizeInv);

	_renderImageShader->bindShaderTextures();

	getRenderScene()->_gBuffer.bind();
	getRenderScene()->_gBuffer.setDrawEffect();
	getRenderScene()->_gBuffer.setReadEffect();

	getRenderScene()->_gBuffer.setViewport();

	getRenderScene()->renderNormalizedQuad();

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}