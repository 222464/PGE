#pragma once

#include "../SFMLOGL.h"

namespace pge {
    class TextureRT {
    private:
        unsigned int width, height;

        GLuint fboID, texID, depthID; // depthID is set to -1 to indicate that it is not used

    public:
        TextureRT()
            : fboID(0)
        {}

        ~TextureRT() {
            if (fboID != 0)
                destroy();
        }

        void create(unsigned int width, unsigned int height, bool depthAttachment, GLuint internalFormat, GLuint textureFormat, GLuint dataType);
        void destroy();

        unsigned int getWidth() const {
            return width;
        }

        unsigned int getHeight() const {
            return height;
        }

        void setViewport() {
            glViewport(0, 0, width, height);
        }

        void bind() {
            glBindFramebuffer(GL_FRAMEBUFFER, fboID);
        }

        void bindDraw() {
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboID);
        }

        void bindRead() {
            glBindFramebuffer(GL_READ_FRAMEBUFFER, fboID);
        }

        static void unbind() {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        static void unbindDraw() {
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        }

        static void unbindRead() {
            glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        }

        GLuint getTextureID() const {
            return texID;
        }

        GLuint getFBOID() const {
            return fboID;
        }

        void releaseTexture() {
            texID = 0;
        }

        bool created() const {
            return fboID != 0;
        }
    };
}