#include <pge/rendering/GBuffer.h>

#include <pge/scene/RenderScene.h>

#include <assert.h>

using namespace pge;

GBuffer::~GBuffer() {
	if (_fboID != 0) {
		glDeleteFramebuffers(1, &_fboID);
		glDeleteTextures(_numBufferTextures, &_gTextureIDs[0]);
		glDeleteTextures(1, &_depthTextureID);
		glDeleteTextures(1, &_effectTextureID);
	}
}

void GBuffer::create(unsigned int width, unsigned int height) {
	_width = width;
	_height = height;

	// Create the FBO
	glGenFramebuffers(1, &_fboID);
	glBindFramebuffer(GL_FRAMEBUFFER, _fboID);

	// Create all GBuffer textures
	glGenTextures(_numBufferTextures, &_gTextureIDs[0]);

	// Position
	glBindTexture(GL_TEXTURE_2D, _gTextureIDs[_positionAndEmissive]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, _width, _height, 0, GL_RGBA, GL_HALF_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Normal
	glBindTexture(GL_TEXTURE_2D, _gTextureIDs[_normalAndShininess]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, _width, _height, 0, GL_RGBA, GL_HALF_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Color
	glBindTexture(GL_TEXTURE_2D, _gTextureIDs[_diffuseAndSpecular]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, _width, _height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Attach the textures to the FBO
	for (GLuint i = 0; i < _numBufferTextures; i++)
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, _gTextureIDs[i], 0);

	// Create depth texture with stencil
	glGenTextures(1, &_depthTextureID);
	glBindTexture(GL_TEXTURE_2D, _depthTextureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, _width, _height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);

	// Make it readable in a shader
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
	glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE);

	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, _depthTextureID, 0);

	// Create the effect buffer
	glGenTextures(1, &_effectTextureID);
	glBindTexture(GL_TEXTURE_2D, _effectTextureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, _width, _height, 0, GL_RGB, GL_HALF_FLOAT, nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + _effectTextureAttachment, GL_TEXTURE_2D, _effectTextureID, 0);

	// Set which buffers to draw to
	for (GLuint i = 0; i < _numBufferTextures; i++)
		_drawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;

	glDrawBuffers(_numBufferTextures, &_drawBuffers[0]);

	// Check that the buffer was properly created
#ifdef PGE_DEBUG
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cerr << "Could not created FBO!" << std::endl;
#endif

	// Unbind
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	checkForGLError();
}

void GBuffer::copyEffectToMainFramebuffer() {
	unbindDraw();
	bindRead();

	setReadEffect();

	glBlitFramebuffer(0, 0, _width, _height, 0, 0, _width, _height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

void GBuffer::copyEffectToRenderTarget(TextureRT &target) {
	target.bindDraw();
	bindRead();

	setReadEffect();

	glBlitFramebuffer(0, 0, _width, _height, 0, 0, _width, _height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}