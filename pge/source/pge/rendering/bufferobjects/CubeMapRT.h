#pragma once

#include "../SFMLOGL.h"

#include "../../constructs/Quaternion.h"

#include <assert.h>

namespace pge {
    class CubeMapRT {
    private:
        GLuint fboID;
        GLuint cubeMapID;

        GLuint depthID;

        unsigned int resolution;

        std::array<Quaternion, 6> baseViewRotations;

    public:
        CubeMapRT()
            : fboID(0)
        {}

        ~CubeMapRT() {
            if (created())
                destroy();
        }

        void create(unsigned int resolution, GLuint internalFormat, GLuint textureFormat, GLuint dataType);
        void destroy();

        void renderFace(class RenderScene* pRenderScene, const Vec3f &position, unsigned char face, float zNear, float zFar, float distance);

        unsigned int getResolution() const {
            return resolution;
        }

        void setViewport() {
            glViewport(0, 0, resolution, resolution);
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
            return cubeMapID;
        }

        GLuint getFBOID() const {
            return fboID;
        }

        bool created() const {
            return fboID != 0;
        }
    };
}