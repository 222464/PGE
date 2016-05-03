#include <pge/rendering/imageeffects/SceneObjectFog.h>

using namespace pge;

void SceneObjectFog::create(const std::shared_ptr<Shader> &fogShader) {
	_fogShader = fogShader;

	Vec2f gBufferSizeInv(1.0f / static_cast<float>(getRenderScene()->_gBuffer.getWidth()), 1.0f / static_cast<float>(getRenderScene()->_gBuffer.getHeight()));

	_fogShader->bind();

	_fogShader->setShaderTexture("pgeGBufferPosition", getRenderScene()->_gBuffer.getTextureID(GBuffer::_positionAndEmissive), GL_TEXTURE_2D);
	_fogShader->setUniformv2f("pgeSizeInv", gBufferSizeInv);
}

void SceneObjectFog::postRender() {
	glDisable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR);

	// Render luma to full ping
	_fogShader->bind();
	_fogShader->setUniformv3f("pgeFogColor", _fogColor);
	_fogShader->setUniformf("pgeFogStartDistance", _fogStartDistance);

	_fogShader->bindShaderTextures();

	getRenderScene()->renderNormalizedQuad();

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}