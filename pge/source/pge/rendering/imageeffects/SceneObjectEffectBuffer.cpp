#include "SceneObjectEffectBuffer.h"

using namespace pge;

void SceneObjectEffectBuffer::create(float downsampleRatio) {
	assert(getRenderScene() != nullptr);

	unsigned int downsampleWidth = static_cast<unsigned int>(getRenderScene()->gBuffer.getWidth() * downsampleRatio);
	unsigned int downsampleHeight = static_cast<unsigned int>(getRenderScene()->gBuffer.getHeight() * downsampleRatio);

	ping.reset(new TextureRT());
	pong.reset(new TextureRT());

	ping->create(downsampleWidth, downsampleHeight, false, GL_RGB16F, GL_RGB, GL_FLOAT);
	pong->create(downsampleWidth, downsampleHeight, false, GL_RGB16F, GL_RGB, GL_FLOAT);

	glBindTexture(GL_TEXTURE_2D, ping->getTextureID());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, pong->getTextureID());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	fullPing.reset(new TextureRT());
	fullPong.reset(new TextureRT());

	fullPing->create(getRenderScene()->gBuffer.getWidth(), getRenderScene()->gBuffer.getHeight(), false, GL_RGB16F, GL_RGB, GL_HALF_FLOAT);
	fullPong->create(getRenderScene()->gBuffer.getWidth(), getRenderScene()->gBuffer.getHeight(), false, GL_RGB16F, GL_RGB, GL_HALF_FLOAT);

	glBindTexture(GL_TEXTURE_2D, fullPing->getTextureID());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, fullPong->getTextureID());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}
