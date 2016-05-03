#pragma once

#include <pge/rendering/SFMLOGL.h>

namespace pge {
	class DepthRT {
	public:
		enum Precision {
			_16, _32
		};

	private:
		GLuint _fboID;
		GLuint _depthID;

		unsigned int _width, _height;

	public:
		DepthRT()
			: _fboID(0)
		{}

		~DepthRT() {
			if (_fboID != 0)
				destroy();
		}

		void create(unsigned int width, unsigned int height, Precision precision = _16);

		unsigned int getWidth() const {
			return _width;
		}

		unsigned int getHeight() const {
			return _height;
		}

		void setViewport() {
			glViewport(0, 0, _width, _height);
		}

		void bind() {
			glBindFramebuffer(GL_FRAMEBUFFER, _fboID);
		}

		void bindDraw() {
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fboID);
		}

		void bindRead() {
			glBindFramebuffer(GL_READ_FRAMEBUFFER, _fboID);
		}

		static void unbind() {
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		static void unbindDraw() {
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		}

		static void unbindRead() {
			glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
		}

		GLuint getDepthTextureID() const {
			return _depthID;
		}

		GLuint getFBOID() const {
			return _fboID;
		}

		void destroy();
	};
}