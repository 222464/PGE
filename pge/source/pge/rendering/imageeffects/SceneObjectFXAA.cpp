#include "SceneObjectFXAA.h"

using namespace pge;

void SceneObjectFXAA::create(const std::shared_ptr<Shader> &fxaaShader, const std::shared_ptr<Shader> &lumaShader) {
	renderMask = 0xffff;

	this->fxaaShader = fxaaShader;
	this->lumaShader = lumaShader;

	lumaBuffer.reset(new TextureRT());
	lumaBuffer->create(getRenderScene()->gBuffer.getWidth(), getRenderScene()->gBuffer.getHeight(), false, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);

	Vec2f gBufferSizeInv(1.0f / static_cast<float>(getRenderScene()->gBuffer.getWidth()), 1.0f / static_cast<float>(getRenderScene()->gBuffer.getHeight()));

	fxaaShader->bind();
	fxaaShader->setShaderTexture("pgeTex", lumaBuffer->getTextureID(), GL_TEXTURE_2D);
	fxaaShader->setUniformv2f("pgeRcpFrame", gBufferSizeInv);
	fxaaShader->setUniformv4f("pgeRcpFrameOpt", Vec4f(2.0f * gBufferSizeInv.x, 2.0f * gBufferSizeInv.y, 0.5f * gBufferSizeInv.x, 0.5f * gBufferSizeInv.y));

	lumaShader->bind();
	lumaShader->setShaderTexture("pgeScene", getRenderScene()->gBuffer.getEffectTextureID(), GL_TEXTURE_2D);
	lumaShader->setUniformv2f("pgeSizeInv", gBufferSizeInv);
}

void SceneObjectFXAA::postRender() {
	glDisable(GL_DEPTH_TEST);

	// Render luma to full ping
	lumaShader->bind();
	lumaShader->bindShaderTextures();

	lumaBuffer->bind();

	getRenderScene()->renderNormalizedQuad();

	fxaaShader->bind();
	fxaaShader->bindShaderTextures();

	getRenderScene()->gBuffer.bind();
	getRenderScene()->gBuffer.setDrawEffect();
	getRenderScene()->gBuffer.setReadEffect();

	getRenderScene()->renderNormalizedQuad();

	glEnable(GL_DEPTH_TEST);
}