#include "SFMLOGL.h"

#include <GL/glu.h>

#include <iostream>

using namespace pge;

GLuint pge::vaoID = 0;

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
        GLenum err = glewInit();

        if (GLEW_OK != err)
            fprintf(stderr, "Error: %s\n", glewGetErrorString(err));

	glGenVertexArrays(1, &vaoID);
    glBindVertexArray(vaoID);

	glFrontFace(GL_CCW);

	glEnable(GL_CULL_FACE);

	glClearDepth(1.0f);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);
}
