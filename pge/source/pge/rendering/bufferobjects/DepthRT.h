#pragma once

#include "../SFMLOGL.h"

namespace pge {
	class DepthRT {
	public:
		enum Precision {
			_16, _32
		};

	private:
		GLuint fboID;
		GLuint depthID;

		unsigned int width, height;

	public:
		DepthRT()
			: fboID(0)
		{}

		~DepthRT() {
			if (fboID != 0)
				destroy();
		}

		void create(unsigned int width, unsigned int height, Precision precision = _16);

		unsigned int getWidth() const {
			return width;
		}

		unsigned int getHeight() const {
			return height;
		}

		void setViewport() {
			glViewport(0, 0, width, height);
		}

		void bind() {
			glBindFramebuffer(GL_FRAMEBUFFER, fboID);
		}

		void bindDraw() {
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboID);
		}

		void bindRead() {
			glBindFramebuffer(GL_READ_FRAMEBUFFER, fboID);
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
			return depthID;
		}

		GLuint getFBOID() const {
			return fboID;
		}

		void destroy();
	};
}