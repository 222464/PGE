#include <pge/rendering/SFMLOGL.h>

#include <iostream>

using namespace pge;

GLuint pge::_vaoID = 0;

bool pge::checkForGLError() {
	GLuint errorCode = glGetError();

	if (errorCode != GL_NO_ERROR) {
		std::cerr << gluErrorString(errorCode) << std::endl;
		abort();

		return true;
	}

	return false;
}

void pge::sfmloglSetup() {
	glGenVertexArrays(1, &_vaoID);
	glBindVertexArray(_vaoID);

	glEnableVertexAttribArray(D3D_ATTRIB_POSITION);
	glEnableVertexAttribArray(D3D_ATTRIB_NORMAL);
	glEnableVertexAttribArray(D3D_ATTRIB_TEXCOORD);

	glFrontFace(GL_CCW);

	glEnable(GL_CULL_FACE);

	glClearDepth(1.0f);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);
}