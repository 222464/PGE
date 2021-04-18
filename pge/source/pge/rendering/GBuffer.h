#pragma once

#include "SFMLOGL.h"
#include "shader/Shader.h"
#include "bufferobjects/TextureRT.h"

#include "../system/Uncopyable.h"

namespace pge {
    class GBuffer : public Uncopyable {
    private:
        unsigned int width, height;

        // Array contains textures for the different buffer types
        static const unsigned int numBufferTextures = 3;
        static const unsigned int numBuffersAndEffect = numBufferTextures + 1;
        static const unsigned int effectTextureAttachment = numBufferTextures;

        std::array<GLuint, numBufferTextures> drawBuffers;
        std::array<GLuint, numBufferTextures> gTextureIDs;

        GLuint fboID;
        GLuint depthTextureID;
        GLuint effectTextureID;

    public:
        enum BufferType {
            positionAndEmissive = 0, normalAndShininess, diffuseAndSpecular
        };

        GBuffer()
            : fboID(0)
        {}

        ~GBuffer();

        void create(unsigned int width, unsigned int height);

        void setDrawGeom() {
            glDrawBuffers(numBufferTextures, &drawBuffers[0]);
        }

        void setDrawBuffer(BufferType type) {
            glDrawBuffer(GL_COLOR_ATTACHMENT0 + type);
        }

        void setReadBuffer(BufferType type) {
            glReadBuffer(GL_COLOR_ATTACHMENT0 + type);
        }

        void setDrawEffect() {
            glDrawBuffer(GL_COLOR_ATTACHMENT0 + effectTextureAttachment);
        }

        void setReadEffect() {
            glReadBuffer(GL_COLOR_ATTACHMENT0 + effectTextureAttachment);
        }

        void copyEffectToMainFramebuffer();
        void copyEffectToRenderTarget(TextureRT &target);

        GLuint getTextureID(BufferType type) const {
            return gTextureIDs[type];
        }

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

        GLuint getFBOID() const {
            return fboID;
        }

        bool created() const {
            return fboID != 0;
        }

        GLuint getEffectTextureID() const {
            return effectTextureID;
        }

        GLuint getDepthTextureID() const {
            return depthTextureID;
        }
    };
}
