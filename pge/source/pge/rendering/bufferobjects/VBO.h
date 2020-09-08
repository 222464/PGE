#pragma once

#include "../SFMLOGL.h"

#include <array>

#include <assert.h>

namespace pge {
	class VBO {
	private:
		GLuint ID;

		GLuint usage;

		static GLuint currentArrayBuffer;
		static GLuint currentElementArrayBuffer;
	
	public:
		VBO()
			: ID(0)
		{}

		~VBO() {
			if (ID != 0)
				glDeleteBuffers(1, &ID);
		}

		void create() {
			assert(ID == 0);

			glGenBuffers(1, &ID);
		}

		void destroy();

		void bind(GLuint usage);
		void forceBind(GLuint usage);

		void unbind();

		static void unbind(GLuint usage);

		static GLuint getCurrentArrayBuffer() {
			return currentArrayBuffer;
		}

		static GLuint getCurrentElementArrayBuffer() {
			return currentElementArrayBuffer;
		}

		bool created() const {
			return ID != 0;
		}

		GLuint getID() const {
			return ID;
		}

		GLuint getUsage() const {
			return usage;
		}
	};
}