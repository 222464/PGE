#include <pge/rendering/bufferobjects/CubeMapDepthRT.h>

#include <pge/scene/RenderScene.h>

#include <pge/util/Math.h>

using namespace pge;

void CubeMapDepthRT::create(unsigned int resolution, Precision precision) {
#ifdef PGE_DEBUG
	int result;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &result);
	size_t uResult = static_cast<unsigned>(result);
	assert(resolution > 0 && resolution <= uResult);
#endif

	_resolution = resolution;

	glGenTextures(1, &_depthCubeMapID);

	glBindTexture(GL_TEXTURE_CUBE_MAP, _depthCubeMapID);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	// For PCF
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);

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
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, precisionEnum, _resolution, _resolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

	// Unbind
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	glGenFramebuffers(1, &_fboID);
	glBindFramebuffer(GL_FRAMEBUFFER, _fboID);

	glDrawBuffer(GL_NONE); // No color buffer
	glReadBuffer(GL_NONE);

	// For completeness
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X, _depthCubeMapID, 0);

	// Check that the buffer was properly created
#ifdef PGE_DEBUG
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cerr << "Could not create depth cube map FBO!" << std::endl;
#endif

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Set up view matrices
	_baseViewRotations[0] = Quaternion::getFromMatrix(Matrix4x4f::cameraDirectionMatrix(Vec3f(1.0f, 0.0f, 0.0f), Vec3f(0.0f, -1.0f, 0.0f)));
	_baseViewRotations[1] = Quaternion::getFromMatrix(Matrix4x4f::cameraDirectionMatrix(Vec3f(-1.0f, 0.0f, 0.0f), Vec3f(0.0f, -1.0f, 0.0f)));
	_baseViewRotations[2] = Quaternion::getFromMatrix(Matrix4x4f::cameraDirectionMatrix(Vec3f(0.0f, 1.0f, 0.0f), Vec3f(0.0f, 0.0f, 1.0f)));
	_baseViewRotations[3] = Quaternion::getFromMatrix(Matrix4x4f::cameraDirectionMatrix(Vec3f(0.0f, -1.0f, 0.0f), Vec3f(0.0f, 0.0f, -1.0f)));
	_baseViewRotations[4] = Quaternion::getFromMatrix(Matrix4x4f::cameraDirectionMatrix(Vec3f(0.0f, 0.0f, 1.0f), Vec3f(0.0f, -1.0f, 0.0f)));
	_baseViewRotations[5] = Quaternion::getFromMatrix(Matrix4x4f::cameraDirectionMatrix(Vec3f(0.0f, 0.0f, -1.0f), Vec3f(0.0f, -1.0f, 0.0f)));

	PGE_GL_ERROR_CHECK();
}

void CubeMapDepthRT::destroy() {
	assert(created());

	glDeleteFramebuffers(1, &_fboID);
	glDeleteTextures(1, &_depthCubeMapID);

	_fboID = 0;
}

void CubeMapDepthRT::renderFace(RenderScene* pRenderScene, const Vec3f &position, unsigned char face, float zNear, float zFar, float distance) {
	assert(created());
	assert(face < 6);

	Camera oldCamera(pRenderScene->_renderCamera);

	pRenderScene->_renderCamera._projectionMatrix = Matrix4x4f::perspectiveMatrix(_piOver2, 1.0f, zNear, zFar);
	pRenderScene->_renderCamera._position = position;

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, _depthCubeMapID, 0);

	glClear(GL_DEPTH_BUFFER_BIT);

	pRenderScene->_renderCamera._rotation = _baseViewRotations[face];

	pRenderScene->_renderCamera.fullUpdate();

	pRenderScene->setTransform(Matrix4x4f::identityMatrix());

	pRenderScene->renderShadow();

	// Revert camera
	pRenderScene->_renderCamera = oldCamera;
}

Camera CubeMapDepthRT::getCamera(unsigned char face, const Vec3f &position, float zNear, float zFar) const {
	Camera camera;

	camera._projectionMatrix = Matrix4x4f::perspectiveMatrix(_piOver2, 1.0f, zNear, zFar);
	camera._position = position;
	camera._rotation = _baseViewRotations[face];

	camera.fullUpdate();

	return camera;
}