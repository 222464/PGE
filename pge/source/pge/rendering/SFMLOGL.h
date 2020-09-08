#pragma once

#include <gl_core_4_4.h>
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>

// Vertex attribute locations
#define PGE_ATTRIB_POSITION 0
#define PGE_ATTRIB_NORMAL 1
#define PGE_ATTRIB_TEXCOORD 2

// So only runs debug function when in debug mode
#ifdef PGE_DEBUG
#define PGE_GL_ERROR_CHECK() pge::checkForGLError()
#else
#define PGE_GL_ERROR_CHECK()
#endif

namespace pge {
	extern GLuint vaoID;
	bool checkForGLError();

	void sfmloglSetup();
}