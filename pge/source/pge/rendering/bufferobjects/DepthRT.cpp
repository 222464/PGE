#include "DepthRT.h"

#include <iostream>

#include <assert.h>

using namespace pge;

void DepthRT::create(unsigned int width, unsigned int height, Precision precision) {
	this->width = width;
	this->height = height;

#ifdef PGE_DEBUG
	int result;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &result);
	size_t uResult = static_cast<unsigned>(result);
	assert(width > 0 && height > 0 && width <= uResult && height <= uResult);
#endif

	glGenFramebuffers(1, &fboID);
	glBindFramebuffer(GL_FRAMEBUFFER, fboID);
 
	glGenTextures(1, &depthID);
	glBindTexture(GL_TEXTURE_2D, depthID);

	switch(precision) {
	case _16:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		break;
	case _32:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		break;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	// For PCF
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthID, 0);
 
	glDrawBuffer(GL_NONE); // No color buffer
	glReadBuffer(GL_NONE);

	// Check that the buffer was properly created
#ifdef PGE_DEBUG
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cerr << "Could not create depth RT!" << std::endl;
#endif

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DepthRT::destroy() {
	assert(fboID != 0);

	glDeleteFramebuffers(1, &fboID);
	glDeleteTextures(1, &depthID);

	fboID = 0;
}
