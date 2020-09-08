#include "CubeMapRT.h"

#include "../../scene/RenderScene.h"

#include "../../util/Math.h"

using namespace pge;

void CubeMapRT::create(unsigned int resolution, GLuint internalFormat, GLuint textureFormat, GLuint dataType) {
#ifdef PGE_DEBUG
	int result;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &result);
	size_t uResult = static_cast<unsigned>(result);
	assert(resolution > 0 && resolution <= uResult);
#endif

	this->resolution = resolution;

	glGenTextures(1, &cubeMapID);

	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapID);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// Create all faces
	for(unsigned int i = 0; i < 6; i++)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, resolution, resolution, 0, textureFormat, dataType, NULL);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	glGenFramebuffers(1, &fboID);
	glBindFramebuffer(GL_FRAMEBUFFER, fboID);

	for(unsigned int i = 0; i < 6; i++)
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cubeMapID, 0);

	// Render buffer creation for depth buffer
	glGenRenderbuffers(1, &depthID);
	glBindRenderbuffer(GL_RENDERBUFFER, depthID);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, resolution, resolution);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthID);

	// Unbind
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	// Check that the buffer was properly created
#ifdef PGE_DEBUG
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cerr << "Could not create cube map FBO!" << std::endl;
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

void CubeMapRT::destroy() {
	assert(created());

	glDeleteFramebuffers(1, &fboID);
	glDeleteTextures(1, &cubeMapID);

	glDeleteRenderbuffers(1, &depthID);

	fboID = 0;
}

void CubeMapRT::renderFace(RenderScene* pRenderScene, const Vec3f &position, unsigned char face, float zNear, float zFar, float distance) {
	assert(created());
	assert(face < 6);

	Camera oldCamera(pRenderScene->renderCamera);

	pRenderScene->renderCamera.projectionMatrix = Matrix4x4f::perspectiveMatrix(piOver2, 1.0f, zNear, zFar);
	pRenderScene->renderCamera.position = position;

	glDrawBuffer(GL_COLOR_ATTACHMENT0 + face);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	pRenderScene->renderCamera.rotation = baseViewRotations[face];

	pRenderScene->renderCamera.fullUpdate();

	pRenderScene->setTransform(Matrix4x4f::identityMatrix());

	pRenderScene->renderShadow();

	// Revert camera
	pRenderScene->renderCamera = oldCamera;
}