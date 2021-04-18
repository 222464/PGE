#pragma once

#include "../SFMLOGL.h"

#include "../../constructs/Quaternion.h"

#include "../Camera.h"

#include <assert.h>

namespace pge {
    class CubeMapDepthRT {
    public:
        enum Precision {
            _16, _32
        };
    private:
        GLuint fboID;
        GLuint depthCubeMapID;

        unsigned int resolution;

        std::array<Quaternion, 6> baseViewRotations;

    public:
        CubeMapDepthRT()
            : fboID(0)
        {}

        ~CubeMapDepthRT() {
            if (created())
                destroy();
        }

        void create(unsigned int resolution, Precision precision = _16);
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

        GLuint getDepthTextureID() const {
            return depthCubeMapID;
        }

        GLuint getFBOID() const {
            return fboID;
        }

        bool created() const {
            return fboID != 0;
        }

        Camera getCamera(unsigned char face, const Vec3f &position, float zNear, float zFar) const;
    };
}