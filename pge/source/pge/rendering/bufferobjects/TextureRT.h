#pragma once

#include <pge/rendering/SFMLOGL.h>

namespace pge {
	class TextureRT {
	private:
		unsigned int _width, _height;

		GLuint _fboID, _texID, _depthID; // _depthID is set to -1 to indicate that it is not used

	public:
		TextureRT()
			: _fboID(0)
		{}

		~TextureRT() {
			if (_fboID != 0)
				destroy();
		}

		void create(unsigned int width, unsigned int height, bool depthAttachment, GLuint internalFormat, GLuint textureFormat, GLuint dataType);
		void destroy();

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

		GLuint getTextureID() const {
			return _texID;
		}

		GLuint getFBOID() const {
			return _fboID;
		}

		void releaseTexture() {
			_texID = 0;
		}

		bool created() const {
			return _fboID != 0;
		}
	};
}