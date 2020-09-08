#include "VBO.h"

using namespace pge;

GLuint VBO::currentArrayBuffer = 0;
GLuint VBO::currentElementArrayBuffer = 0;

void VBO::destroy() {
	assert(ID != 0);

	switch (usage) {
	case GL_ARRAY_BUFFER:
		if (currentArrayBuffer == ID) {
			currentArrayBuffer = 0;
			glBindBuffer(usage, 0);
		}

		break;

	case GL_ELEMENT_ARRAY_BUFFER:
		if (currentElementArrayBuffer == ID) {
			currentElementArrayBuffer = 0;
			glBindBuffer(usage, 0);
		}

		break;
	}

	glDeleteBuffers(1, &ID);

	ID = 0;
}

void VBO::bind(GLuint usage) {
	this->usage = usage;

	/*switch (usage) {
	case GL_ARRAY_BUFFER:
		if (currentArrayBuffer != ID) {
			currentArrayBuffer = ID;
			glBindBuffer(usage, ID);
		}

		break;

	case GL_ELEMENT_ARRAY_BUFFER:
		if (currentElementArrayBuffer != ID) {
			currentElementArrayBuffer = ID;
			glBindBuffer(usage, ID);
		}

		break;

	default:
		glBindBuffer(usage, ID);

		break;
	}*/

	glBindBuffer(usage, ID);
}

void VBO::forceBind(GLuint usage) {
	this->usage = usage;

	switch (usage) {
	case GL_ARRAY_BUFFER:
		currentArrayBuffer = ID;

		break;

	case GL_ELEMENT_ARRAY_BUFFER:
		currentElementArrayBuffer = ID;

		break;
	}

	glBindBuffer(usage, ID);
}

void VBO::unbind() {
	glBindBuffer(usage, 0);

	switch (usage) {
	case GL_ARRAY_BUFFER:
		currentArrayBuffer = 0;

		break;

	case GL_ELEMENT_ARRAY_BUFFER:
		currentElementArrayBuffer = 0;

		break;
	}
}

void VBO::unbind(GLuint usage) {
	glBindBuffer(usage, 0);

	switch (usage) {
	case GL_ARRAY_BUFFER:
		currentArrayBuffer = 0;

		break;

	case GL_ELEMENT_ARRAY_BUFFER:
		currentElementArrayBuffer = 0;

		break;
	}
}