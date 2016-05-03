#include <pge/rendering/imageeffects/SceneObjectSSR.h>

using namespace pge;

void SceneObjectSSR::create(const std::shared_ptr<Shader> &blurShaderHorizontalEdgeAware,
	const std::shared_ptr<Shader> &blurShaderVerticalEdgeAware,
	const std::shared_ptr<Shader> &ssrShader,
	const std::shared_ptr<Shader> &renderImageShader,
	const std::shared_ptr<TextureCube> &cubeMap,
	const std::shared_ptr<Texture2D> &noiseMap)
{
	_renderMask = 0xffff;

	_blurShaderHorizontalEdgeAware = blurShaderHorizontalEdgeAware;
	_blurShaderVerticalEdgeAware = blurShaderVerticalEdgeAware;
	_ssrShader = ssrShader;
	_renderImageShader = renderImageShader;
	_cubeMap = cubeMap;
	_noiseMap = noiseMap;

	_ssrShader->bind();

	_ssrShader->setShaderTexture("pgeGBufferColor", getRenderScene()->_gBuffer.getTextureID(GBuffer::_diffuseAndSpecular), GL_TEXTURE_2D);
	_ssrShader->setShaderTexture("pgeGBufferPosition", getRenderScene()->_gBuffer.getTextureID(GBuffer::_positionAndEmissive), GL_TEXTURE_2D);
	_ssrShader->setShaderTexture("pgeGBufferNormal", getRenderScene()->_gBuffer.getTextureID(GBuffer::_normalAndShininess), GL_TEXTURE_2D);
	_ssrShader->setShaderTexture("pgeGBufferEffect", getRenderScene()->_gBuffer.getEffectTextureID(), GL_TEXTURE_2D);
	_ssrShader->setShaderTexture("pgeCubeMap", _cubeMap->getTextureID(), GL_TEXTURE_CUBE_MAP);
	_ssrShader->setShaderTexture("pgeNoiseMap", _noiseMap->getTextureID(), GL_TEXTURE_2D);

	_blurShaderHorizontalEdgeAware->bind();

	_blurShaderHorizontalEdgeAware->setShaderTexture("pgeGBufferPosition", getRenderScene()->_gBuffer.getTextureID(GBuffer::_positionAndEmissive), GL_TEXTURE_2D);
	_blurShaderHorizontalEdgeAware->setShaderTexture("pgeGBufferNormal", getRenderScene()->_gBuffer.getTextureID(GBuffer::_normalAndShininess), GL_TEXTURE_2D);

	_blurShaderVerticalEdgeAware->bind();

	_blurShaderVerticalEdgeAware->setShaderTexture("pgeGBufferPosition", getRenderScene()->_gBuffer.getTextureID(GBuffer::_positionAndEmissive), GL_TEXTURE_2D);
	_blurShaderVerticalEdgeAware->setShaderTexture("pgeGBufferNormal", getRenderScene()->_gBuffer.getTextureID(GBuffer::_normalAndShininess), GL_TEXTURE_2D);
}

void SceneObjectSSR::onAdd() {
	_effectBuffer = getScene()->getNamed("ebuf");

	assert(_effectBuffer.isAlive());
}

void SceneObjectSSR::postRender() {
	SceneObjectEffectBuffer* pEffectBuffer = static_cast<SceneObjectEffectBuffer*>(_effectBuffer.get());

	Vec2f sizeInv(1.0f / static_cast<float>(pEffectBuffer->_fullPing->getWidth()), 1.0f / static_cast<float>(pEffectBuffer->_fullPing->getHeight()));

	glDisable(GL_DEPTH_TEST);

	_ssrShader->bind();

	Matrix3x3f normalMatrixInv;
	//Matrix4x4f projectionMatrixInv;

	getRenderScene()->_logicCamera.getNormalMatrix().inverse(normalMatrixInv);
	//getRenderScene()->_logicCamera._projectionMatrix.inverse(projectionMatrixInv);

	_ssrShader->setUniformmat4("pgeProjectionMatrix", getRenderScene()->_logicCamera._projectionMatrix);
	//_ssrShader->setUniformmat4("pgeProjectionMatrixInv", projectionMatrixInv);
	_ssrShader->setUniformmat3("pgeNormalMatrixInv", normalMatrixInv);
	_ssrShader->setUniformv2f("pgeSizeInv", sizeInv);

	_ssrShader->bindShaderTextures();

	pEffectBuffer->_fullPing->bind();
	pEffectBuffer->_fullPing->setViewport();

	getRenderScene()->renderNormalizedQuad();

	for (size_t i = 0; i < _numBlurPasses; i++) {
		_blurShaderHorizontalEdgeAware->bind();

		_blurShaderHorizontalEdgeAware->setShaderTexture("pgeScene", pEffectBuffer->_fullPing->getTextureID(), GL_TEXTURE_2D);
		_blurShaderHorizontalEdgeAware->setUniformf("pgeBlurSize", _blurRadius);
		_blurShaderHorizontalEdgeAware->setUniformv2f("pgeSizeInv", sizeInv);

		_blurShaderHorizontalEdgeAware->bindShaderTextures();

		pEffectBuffer->_fullPong->bind();
		pEffectBuffer->_fullPong->setViewport();

		getRenderScene()->renderNormalizedQuad();

		_blurShaderVerticalEdgeAware->bind();

		_blurShaderVerticalEdgeAware->setShaderTexture("pgeScene", pEffectBuffer->_fullPong->getTextureID(), GL_TEXTURE_2D);
		_blurShaderVerticalEdgeAware->setUniformf("pgeBlurSize", _blurRadius);
		_blurShaderVerticalEdgeAware->setUniformv2f("pgeSizeInv", sizeInv);

		_blurShaderVerticalEdgeAware->bindShaderTextures();

		pEffectBuffer->_fullPing->bind();
		pEffectBuffer->_fullPing->setViewport();

		getRenderScene()->renderNormalizedQuad();
	}

	Vec2f gBufferSizeInv(1.0f / static_cast<float>(getRenderScene()->_gBuffer.getWidth()), 1.0f / static_cast<float>(getRenderScene()->_gBuffer.getHeight()));

	// Blend with effect buffer
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	_renderImageShader->bind();

	_renderImageShader->setShaderTexture("pgeScene", pEffectBuffer->_fullPing->getTextureID(), GL_TEXTURE_2D);

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