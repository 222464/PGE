#include "SceneObjectFog.h"

using namespace pge;

void SceneObjectFog::create(const std::shared_ptr<Shader> &fogShader) {
	this->fogShader = fogShader;

	Vec2f gBufferSizeInv(1.0f / static_cast<float>(getRenderScene()->gBuffer.getWidth()), 1.0f / static_cast<float>(getRenderScene()->gBuffer.getHeight()));

	fogShader->bind();

	fogShader->setShaderTexture("pgeGBufferPosition", getRenderScene()->gBuffer.getTextureID(GBuffer::positionAndEmissive), GL_TEXTURE_2D);
	fogShader->setUniformv2f("pgeSizeInv", gBufferSizeInv);
}

void SceneObjectFog::postRender() {
	glDisable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR);

	// Render luma to full ping
	fogShader->bind();
	fogShader->setUniformv3f("pgeFogColor", fogColor);
	fogShader->setUniformf("pgeFogStartDistance", fogStartDistance);

	fogShader->bindShaderTextures();

	getRenderScene()->renderNormalizedQuad();

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}