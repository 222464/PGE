#include "TextureRT.h"

#include <assert.h>

#include <iostream>

using namespace pge;

void TextureRT::create(unsigned int width, unsigned int height, bool useDepthAttachment, GLuint internalFormat, GLuint textureFormat, GLuint dataType) {
    this->width = width;
    this->height = height;

#ifdef PGE_DEBUG
    int result;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &result);
    size_t uResult = static_cast<unsigned>(result);
    assert(width > 0 && height > 0 && width <= uResult && height <= uResult);
#endif

    // Create a texture to attach to it
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    // Default settings for the texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    // Create empty texture
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, textureFormat, dataType, nullptr);

    PGE_GL_ERROR_CHECK();

    // Unbind texture
    glBindTexture(GL_TEXTURE_2D, 0);

    // Generate the FBO
    glGenFramebuffers(1, &fboID);

    glBindFramebuffer(GL_FRAMEBUFFER, fboID);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texID, 0); // The 0 is for mip map levels, we aren't using any

    PGE_GL_ERROR_CHECK();

    // If depth buffer attachment was desired
    if (useDepthAttachment) {
        glGenRenderbuffers(1, &depthID);

        // Bind it so we can set it up
        glBindRenderbuffer(GL_RENDERBUFFER, depthID);

        // Set up the depth buffer
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);

        // Attach the dpeth buffer
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthID);

        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        PGE_GL_ERROR_CHECK();
    }
    else
        depthID = 0; // Unused

    // Check that the buffer was properly created
#ifdef PGE_DEBUG
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "Could not create FBO!" << std::endl;
#endif

    // Unbind
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void TextureRT::destroy() {
    assert(fboID != 0);

    glDeleteFramebuffers(1, &fboID);

    if(texID != 0)
        glDeleteTextures(1, &texID);

    // If depth attachment was used
    if(depthID != 0)
        glDeleteRenderbuffers(1, &depthID);

    fboID = 0;
}