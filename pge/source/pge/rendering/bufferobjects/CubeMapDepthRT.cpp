#include "CubeMapDepthRT.h"

#include "../../scene/RenderScene.h"

#include "../../util/Math.h"

using namespace pge;

void CubeMapDepthRT::create(unsigned int resolution, Precision precision) {
#ifdef PGE_DEBUG
    int result;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &result);
    size_t uResult = static_cast<unsigned>(result);
    assert(resolution > 0 && resolution <= uResult);
#endif

    this->resolution = resolution;

    glGenTextures(1, &depthCubeMapID);

    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubeMapID);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // For PCF
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

    GLenum precisionEnum;

    switch (precision) {
    case _16:
        precisionEnum = GL_DEPTH_COMPONENT16;
        break;
    case _32:
        precisionEnum = GL_DEPTH_COMPONENT32;
        break;
    }

    // Create all faces
    for (unsigned char i = 0; i < 6; i++)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, precisionEnum, resolution, resolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    // Unbind
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    glGenFramebuffers(1, &fboID);
    glBindFramebuffer(GL_FRAMEBUFFER, fboID);

    glDrawBuffer(GL_NONE); // No color buffer
    glReadBuffer(GL_NONE);

    // For completeness
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X, depthCubeMapID, 0);

    // Check that the buffer was properly created
#ifdef PGE_DEBUG
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "Could not create depth cube map FBO!" << std::endl;
#endif

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Set up view matrices
    baseViewRotations[0] = Quaternion::getFromMatrix(Matrix4x4f::cameraDirectionMatrix(Vec3f(1.0f, 0.0f, 0.0f), Vec3f(0.0f, -1.0f, 0.0f)));
    baseViewRotations[1] = Quaternion::getFromMatrix(Matrix4x4f::cameraDirectionMatrix(Vec3f(-1.0f, 0.0f, 0.0f), Vec3f(0.0f, -1.0f, 0.0f)));
    baseViewRotations[2] = Quaternion::getFromMatrix(Matrix4x4f::cameraDirectionMatrix(Vec3f(0.0f, 1.0f, 0.0f), Vec3f(0.0f, 0.0f, 1.0f)));
    baseViewRotations[3] = Quaternion::getFromMatrix(Matrix4x4f::cameraDirectionMatrix(Vec3f(0.0f, -1.0f, 0.0f), Vec3f(0.0f, 0.0f, -1.0f)));
    baseViewRotations[4] = Quaternion::getFromMatrix(Matrix4x4f::cameraDirectionMatrix(Vec3f(0.0f, 0.0f, 1.0f), Vec3f(0.0f, -1.0f, 0.0f)));
    baseViewRotations[5] = Quaternion::getFromMatrix(Matrix4x4f::cameraDirectionMatrix(Vec3f(0.0f, 0.0f, -1.0f), Vec3f(0.0f, -1.0f, 0.0f)));

    PGE_GL_ERROR_CHECK();
}

void CubeMapDepthRT::destroy() {
    assert(created());

    glDeleteFramebuffers(1, &fboID);
    glDeleteTextures(1, &depthCubeMapID);

    fboID = 0;
}

void CubeMapDepthRT::renderFace(RenderScene* pRenderScene, const Vec3f &position, unsigned char face, float zNear, float zFar, float distance) {
    assert(created());
    assert(face < 6);

    Camera oldCamera(pRenderScene->renderCamera);

    pRenderScene->renderCamera.projectionMatrix = Matrix4x4f::perspectiveMatrix(piOver2, 1.0f, zNear, zFar);
    pRenderScene->renderCamera.position = position;

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, depthCubeMapID, 0);

    glClear(GL_DEPTH_BUFFER_BIT);

    pRenderScene->renderCamera.rotation = baseViewRotations[face];

    pRenderScene->renderCamera.fullUpdate();

    pRenderScene->setTransform(Matrix4x4f::identityMatrix());

    pRenderScene->renderShadow();

    // Revert camera
    pRenderScene->renderCamera = oldCamera;
}

Camera CubeMapDepthRT::getCamera(unsigned char face, const Vec3f &position, float zNear, float zFar) const {
    Camera camera;

    camera.projectionMatrix = Matrix4x4f::perspectiveMatrix(piOver2, 1.0f, zNear, zFar);
    camera.position = position;
    camera.rotation = baseViewRotations[face];

    camera.fullUpdate();

    return camera;
}