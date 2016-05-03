#include <pge/rendering/imageeffects/SceneObjectDepthOfField.h>

using namespace pge;

void SceneObjectDepthOfField::create(const std::shared_ptr<Shader> &depthOfFieldBlurShaderHorizontal,
	const std::shared_ptr<Shader> &depthOfFieldBlurShaderVertical,
	const std::shared_ptr<Shader> &renderImageShader)
{
	_renderMask = 0xffff;

	_depthOfFieldBlurShaderHorizontal = depthOfFieldBlurShaderHorizontal;
	_depthOfFieldBlurShaderVertical = depthOfFieldBlurShaderVertical;
	_renderImageShader = renderImageShader;
}

void SceneObjectDepthOfField::onAdd() {
	_effectBuffer = getScene()->getNamed("ebuf");

	assert(_effectBuffer.isAlive());
}

void SceneObjectDepthOfField::postRender() {
	SceneObjectEffectBuffer* pEffectBuffer = static_cast<SceneObjectEffectBuffer*>(_effectBuffer.get());

	Vec2f gBufferSizeInv(1.0f / static_cast<float>(getRenderScene()->_gBuffer.getWidth()), 1.0f / static_cast<float>(getRenderScene()->_gBuffer.getHeight()));

	glDisable(GL_DEPTH_TEST);

	_renderImageShader->bind();

	_renderImageShader->setShaderTexture("pgeScene", getRenderScene()->_gBuffer.getEffectTextureID(), GL_TEXTURE_2D);

	_renderImageShader->setUniformv2f("pgeGBufferSizeInv", gBufferSizeInv);

	_renderImageShader->bindShaderTextures();

	pEffectBuffer->_fullPing->bind();
	pEffectBuffer->_fullPing->setViewport();

	getRenderScene()->renderNormalizedQuad();

	for (size_t i = 0; i < _numBlurPasses; i++) {
		_depthOfFieldBlurShaderHorizontal->bind();

		_depthOfFieldBlurShaderHorizontal->setShaderTexture("pgeScene", pEffectBuffer->_fullPing->getTextureID(), GL_TEXTURE_2D);
		_depthOfFieldBlurShaderHorizontal->setShaderTexture("pgeGBufferPosition", getRenderScene()->_gBuffer.getTextureID(GBuffer::_positionAndEmissive), GL_TEXTURE_2D);
		_depthOfFieldBlurShaderHorizontal->setUniformf("pgeBlurSize", _blurRadius);
		_depthOfFieldBlurShaderHorizontal->setUniformf("pgeFocalDistance", _focalDistance);
		_depthOfFieldBlurShaderHorizontal->setUniformf("pgeFocalRange", _focalRange);
		_depthOfFieldBlurShaderHorizontal->setUniformv2f("pgeSizeInv", gBufferSizeInv);

		_depthOfFieldBlurShaderHorizontal->bindShaderTextures();

		pEffectBuffer->_fullPong->bind();
		pEffectBuffer->_fullPong->setViewport();

		getRenderScene()->renderNormalizedQuad();

		_depthOfFieldBlurShaderVertical->bind();

		_depthOfFieldBlurShaderVertical->setShaderTexture("pgeScene", pEffectBuffer->_fullPong->getTextureID(), GL_TEXTURE_2D);
		_depthOfFieldBlurShaderVertical->setShaderTexture("pgeGBufferPosition", getRenderScene()->_gBuffer.getTextureID(GBuffer::_positionAndEmissive), GL_TEXTURE_2D);
		_depthOfFieldBlurShaderVertical->setUniformf("pgeBlurSize", _blurRadius);
		_depthOfFieldBlurShaderVertical->setUniformf("pgeFocalDistance", _focalDistance);
		_depthOfFieldBlurShaderVertical->setUniformf("pgeFocalRange", _focalRange);
		_depthOfFieldBlurShaderVertical->setUniformv2f("pgeSizeInv", gBufferSizeInv);

		_depthOfFieldBlurShaderVertical->bindShaderTextures();

		pEffectBuffer->_fullPing->bind();
		pEffectBuffer->_fullPing->setViewport();

		getRenderScene()->renderNormalizedQuad();
	}

	_renderImageShader->bind();

	_renderImageShader->setShaderTexture("pgeScene", pEffectBuffer->_fullPing->getTextureID(), GL_TEXTURE_2D);

	_renderImageShader->setUniformv2f("pgeGBufferSizeInv", gBufferSizeInv);

	_renderImageShader->bindShaderTextures();

	getRenderScene()->_gBuffer.bind();
	getRenderScene()->_gBuffer.setDrawEffect();
	getRenderScene()->_gBuffer.setReadEffect();

	getRenderScene()->_gBuffer.setViewport();

	getRenderScene()->renderNormalizedQuad();

	glEnable(GL_DEPTH_TEST);
}