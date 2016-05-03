#pragma once

#include <GL/glew.h>
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>

// Vertex attribute locations
#define D3D_ATTRIB_POSITION 0
#define D3D_ATTRIB_NORMAL 1
#define D3D_ATTRIB_TEXCOORD 2

// So only runs debug function when in debug mode
#ifdef D3D_DEBUG
#define D3D_GL_ERROR_CHECK() pge::checkForGLError()
#else
#define D3D_GL_ERROR_CHECK()
#endif

namespace pge {
	extern GLuint _vaoID;
	bool checkForGLError();

	void sfmloglSetup();
}