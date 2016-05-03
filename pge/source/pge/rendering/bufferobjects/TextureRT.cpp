#include <pge/rendering/bufferobjects/TextureRT.h>

#include <assert.h>

#include <iostream>

using namespace pge;

void TextureRT::create(unsigned int width, unsigned int height, bool useDepthAttachment, GLuint internalFormat, GLuint textureFormat, GLuint dataType) {
	_width = width;
	_height = height;

#ifdef PGE_DEBUG
	int result;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &result);
	size_t uResult = static_cast<unsigned>(result);
	assert(_width > 0 && _height > 0 && _width <= uResult && _height <= uResult);
#endif

	// Create a texture to attach to it
	glGenTextures(1, &_texID);
	glBindTexture(GL_TEXTURE_2D, _texID);

	// Default settings for the texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// Create empty texture
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, _width, _height, 0, textureFormat, dataType, nullptr);

	PGE_GL_ERROR_CHECK();

	// Unbind texture
	glBindTexture(GL_TEXTURE_2D, 0);

	// Generate the FBO
	glGenFramebuffers(1, &_fboID);

	glBindFramebuffer(GL_FRAMEBUFFER, _fboID);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _texID, 0); // The 0 is for mip map levels, we aren't using any

	PGE_GL_ERROR_CHECK();

	// If depth buffer attachment was desired
	if (useDepthAttachment) {
		glGenRenderbuffers(1, &_depthID);

		// Bind it so we can set it up
		glBindRenderbuffer(GL_RENDERBUFFER, _depthID);

		// Set up the depth buffer
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, _width, _height);

		// Attach the dpeth buffer
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthID);

		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		PGE_GL_ERROR_CHECK();
	}
	else
		_depthID = 0; // Unused

	// Check that the buffer was properly created
#ifdef PGE_DEBUG
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cerr << "Could not create FBO!" << std::endl;
#endif

	// Unbind
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void TextureRT::destroy() {
	assert(_fboID != 0);

	glDeleteFramebuffers(1, &_fboID);

	if(_texID != 0)
		glDeleteTextures(1, &_texID);

	// If depth attachment was used
	if(_depthID != 0)
		glDeleteRenderbuffers(1, &_depthID);

	_fboID = 0;
}