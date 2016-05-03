#include <pge/rendering/bufferobjects/VBO.h>

using namespace pge;

GLuint VBO::_currentArrayBuffer = 0;
GLuint VBO::_currentElementArrayBuffer = 0;

void VBO::destroy() {
	assert(_ID != 0);

	switch (_usage) {
	case GL_ARRAY_BUFFER:
		if (_currentArrayBuffer == _ID) {
			_currentArrayBuffer = 0;
			glBindBuffer(_usage, 0);
		}

		break;

	case GL_ELEMENT_ARRAY_BUFFER:
		if (_currentElementArrayBuffer == _ID) {
			_currentElementArrayBuffer = 0;
			glBindBuffer(_usage, 0);
		}

		break;
	}

	glDeleteBuffers(1, &_ID);

	_ID = 0;
}

void VBO::bind(GLuint usage) {
	_usage = usage;

	/*switch (_usage) {
	case GL_ARRAY_BUFFER:
		if (_currentArrayBuffer != _ID) {
			_currentArrayBuffer = _ID;
			glBindBuffer(_usage, _ID);
		}

		break;

	case GL_ELEMENT_ARRAY_BUFFER:
		if (_currentElementArrayBuffer != _ID) {
			_currentElementArrayBuffer = _ID;
			glBindBuffer(_usage, _ID);
		}

		break;

	default:
		glBindBuffer(_usage, _ID);

		break;
	}*/

	glBindBuffer(_usage, _ID);
}

void VBO::forceBind(GLuint usage) {
	_usage = usage;

	switch (_usage) {
	case GL_ARRAY_BUFFER:
		_currentArrayBuffer = _ID;

		break;

	case GL_ELEMENT_ARRAY_BUFFER:
		_currentElementArrayBuffer = _ID;

		break;
	}

	glBindBuffer(_usage, _ID);
}

void VBO::unbind() {
	glBindBuffer(_usage, 0);

	switch (_usage) {
	case GL_ARRAY_BUFFER:
		_currentArrayBuffer = 0;

		break;

	case GL_ELEMENT_ARRAY_BUFFER:
		_currentElementArrayBuffer = 0;

		break;
	}
}

void VBO::unbind(GLuint usage) {
	glBindBuffer(usage, 0);

	switch (usage) {
	case GL_ARRAY_BUFFER:
		_currentArrayBuffer = 0;

		break;

	case GL_ELEMENT_ARRAY_BUFFER:
		_currentElementArrayBuffer = 0;

		break;
	}
}