#include <pge/rendering/imageeffects/SceneObjectFXAA.h>

using namespace pge;

void SceneObjectFXAA::create(const std::shared_ptr<Shader> &fxaaShader, const std::shared_ptr<Shader> &lumaShader) {
	_renderMask = 0xffff;

	_fxaaShader = fxaaShader;
	_lumaShader = lumaShader;

	_lumaBuffer.reset(new TextureRT());
	_lumaBuffer->create(getRenderScene()->_gBuffer.getWidth(), getRenderScene()->_gBuffer.getHeight(), false, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);

	Vec2f gBufferSizeInv(1.0f / static_cast<float>(getRenderScene()->_gBuffer.getWidth()), 1.0f / static_cast<float>(getRenderScene()->_gBuffer.getHeight()));

	_fxaaShader->bind();
	_fxaaShader->setShaderTexture("pgeTex", _lumaBuffer->getTextureID(), GL_TEXTURE_2D);
	_fxaaShader->setUniformv2f("pgeRcpFrame", gBufferSizeInv);
	_fxaaShader->setUniformv4f("pgeRcpFrameOpt", Vec4f(2.0f * gBufferSizeInv.x, 2.0f * gBufferSizeInv.y, 0.5f * gBufferSizeInv.x, 0.5f * gBufferSizeInv.y));

	_lumaShader->bind();
	_lumaShader->setShaderTexture("pgeScene", getRenderScene()->_gBuffer.getEffectTextureID(), GL_TEXTURE_2D);
	_lumaShader->setUniformv2f("pgeSizeInv", gBufferSizeInv);
}

void SceneObjectFXAA::postRender() {
	glDisable(GL_DEPTH_TEST);

	// Render luma to full ping
	_lumaShader->bind();
	_lumaShader->bindShaderTextures();

	_lumaBuffer->bind();

	getRenderScene()->renderNormalizedQuad();

	_fxaaShader->bind();
	_fxaaShader->bindShaderTextures();

	getRenderScene()->_gBuffer.bind();
	getRenderScene()->_gBuffer.setDrawEffect();
	getRenderScene()->_gBuffer.setReadEffect();

	getRenderScene()->renderNormalizedQuad();

	glEnable(GL_DEPTH_TEST);
}