#include <pge/rendering/bufferobjects/DepthRT.h>

#include <iostream>

#include <assert.h>

using namespace pge;

void DepthRT::create(unsigned int width, unsigned int height, Precision precision) {
	_width = width;
	_height = height;

#ifdef D3D_DEBUG
	int result;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &result);
	size_t uResult = static_cast<unsigned>(result);
	assert(_width > 0 && _height > 0 && _width <= uResult && _height <= uResult);
#endif

	glGenFramebuffers(1, &_fboID);
	glBindFramebuffer(GL_FRAMEBUFFER, _fboID);
 
	glGenTextures(1, &_depthID);
	glBindTexture(GL_TEXTURE_2D, _depthID);

	switch(precision) {
	case _16:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, _width, _height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		break;
	case _32:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, _width, _height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		break;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	// For PCF
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY); 				
 
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, _depthID, 0);
 
	glDrawBuffer(GL_NONE); // No color buffer
	glReadBuffer(GL_NONE);

	// Check that the buffer was properly created
#ifdef D3D_DEBUG
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cerr << "Could not create depth RT!" << std::endl;
#endif

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DepthRT::destroy() {
	assert(_fboID != 0);

	glDeleteFramebuffers(1, &_fboID);
	glDeleteTextures(1, &_depthID);

	_fboID = 0;
}