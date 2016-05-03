#pragma once

#include <pge/rendering/SFMLOGL.h>

#include <array>

#include <assert.h>

namespace pge {
	class VBO {
	private:
		GLuint _ID;

		GLuint _usage;

		static GLuint _currentArrayBuffer;
		static GLuint _currentElementArrayBuffer;
	
	public:
		VBO()
			: _ID(0)
		{}

		~VBO() {
			if (_ID != 0)
				glDeleteBuffers(1, &_ID);
		}

		void create() {
			assert(_ID == 0);

			glGenBuffers(1, &_ID);
		}

		void destroy();

		void bind(GLuint usage);
		void forceBind(GLuint usage);

		void unbind();

		static void unbind(GLuint usage);

		static GLuint getCurrentArrayBuffer() {
			return _currentArrayBuffer;
		}

		static GLuint getCurrentElementArrayBuffer() {
			return _currentElementArrayBuffer;
		}

		bool created() const {
			return _ID != 0;
		}

		GLuint getID() const {
			return _ID;
		}

		GLuint getUsage() const {
			return _usage;
		}
	};
}