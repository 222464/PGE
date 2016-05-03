#include <pge/rendering/imageeffects/SceneObjectEffectBuffer.h>

using namespace pge;

void SceneObjectEffectBuffer::create(float downsampleRatio) {
	assert(getRenderScene() != nullptr);

	unsigned int downsampleWidth = static_cast<unsigned int>(getRenderScene()->_gBuffer.getWidth() * downsampleRatio);
	unsigned int downsampleHeight = static_cast<unsigned int>(getRenderScene()->_gBuffer.getHeight() * downsampleRatio);

	_ping.reset(new TextureRT());
	_pong.reset(new TextureRT());

	_ping->create(downsampleWidth, downsampleHeight, false, GL_RGB16F, GL_RGB, GL_FLOAT);
	_pong->create(downsampleWidth, downsampleHeight, false, GL_RGB16F, GL_RGB, GL_FLOAT);

	glBindTexture(GL_TEXTURE_2D, _ping->getTextureID());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, _pong->getTextureID());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	_fullPing.reset(new TextureRT());
	_fullPong.reset(new TextureRT());

	_fullPing->create(getRenderScene()->_gBuffer.getWidth(), getRenderScene()->_gBuffer.getHeight(), false, GL_RGB16F, GL_RGB, GL_FLOAT);
	_fullPong->create(getRenderScene()->_gBuffer.getWidth(), getRenderScene()->_gBuffer.getHeight(), false, GL_RGB16F, GL_RGB, GL_FLOAT);

	glBindTexture(GL_TEXTURE_2D, _fullPing->getTextureID());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, _fullPong->getTextureID());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}