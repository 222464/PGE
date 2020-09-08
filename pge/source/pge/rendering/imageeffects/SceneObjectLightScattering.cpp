#include "SceneObjectLightScattering.h"

using namespace pge;

void SceneObjectLightScattering::create(const std::shared_ptr<Shader> &blurShaderHorizontal,
	const std::shared_ptr<Shader> &blurShaderVertical,
	const std::shared_ptr<Shader> &lightScatteringShader,
	const std::shared_ptr<Shader> &renderImageShader)
{
	renderMask = 0xffff;

	this->blurShaderHorizontal = blurShaderHorizontal;
	this->blurShaderVertical = blurShaderVertical;
	this->lightScatteringShader = lightScatteringShader;
	this->renderImageShader = renderImageShader;

	lightScatteringShader->bind();

	lightScatteringShader->setShaderTexture("pgeGBufferPosition", getRenderScene()->gBuffer.getTextureID(GBuffer::positionAndEmissive), GL_TEXTURE_2D);
}

void SceneObjectLightScattering::onAdd() {
	effectBuffer = getScene()->getNamed("ebuf");

	assert(effectBuffer.isAlive());
}

void SceneObjectLightScattering::postRender() {
	SceneObjectEffectBuffer* pEffectBuffer = static_cast<SceneObjectEffectBuffer*>(effectBuffer.get());

	Vec2f sizeInv(1.0f / static_cast<float>(pEffectBuffer->ping->getWidth()), 1.0f / static_cast<float>(pEffectBuffer->ping->getHeight()));

	glDisable(GL_DEPTH_TEST);

	lightScatteringShader->bind();

	lightScatteringShader->setUniformv2f("pgeSizeInv", sizeInv);

	lightScatteringShader->setUniformv3f("pgeColor", lightSourceColor * std::max(0.0f, (getScene()->logicCamera.rotation * Vec3f(0.0f, 0.0f, -1.0f)).dot((lightSourcePosition - getScene()->logicCamera.position).normalized())));

	lightScatteringShader->setUniformv4f("pgeLightProjectedPosition", getScene()->logicCamera.getProjectionViewMatrix() * Vec4f(lightSourcePosition.x, lightSourcePosition.y, lightSourcePosition.z, 1.0f));
	lightScatteringShader->setUniformv3f("pgeLightViewPosition", getScene()->logicCamera.getViewMatrix() * lightSourcePosition);

	lightScatteringShader->bindShaderTextures();

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
	glBlendFunc(GL_ONE, GL_ONE);

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