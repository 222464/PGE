#pragma once

#include <pge/rendering/SFMLOGL.h>
#include <pge/rendering/shader/Shader.h>
#include <pge/rendering/bufferobjects/TextureRT.h>

#include <pge/system/Uncopyable.h>

namespace pge {
	class GBuffer : public Uncopyable {
	private:
		unsigned int _width, _height;

		// Array contains textures for the different buffer types
		static const unsigned int _numBufferTextures = 3;
		static const unsigned int _numBuffersAndEffect = _numBufferTextures + 1;
		static const unsigned int _effectTextureAttachment = _numBufferTextures;

		std::array<GLuint, _numBufferTextures> _drawBuffers;
		std::array<GLuint, _numBufferTextures> _gTextureIDs;

		GLuint _fboID;
		GLuint _depthTextureID;
		GLuint _effectTextureID;

	public:
		enum BufferType {
			_positionAndEmissive = 0, _normalAndShininess, _diffuseAndSpecular
		};

		GBuffer()
			: _fboID(0)
		{}

		~GBuffer();

		void create(unsigned int width, unsigned int height);

		void setDrawGeom() {
			glDrawBuffers(_numBufferTextures, &_drawBuffers[0]);
		}

		void setDrawBuffer(BufferType type) {
			glDrawBuffer(GL_COLOR_ATTACHMENT0 + type);
		}

		void setReadBuffer(BufferType type) {
			glReadBuffer(GL_COLOR_ATTACHMENT0 + type);
		}

		void setDrawEffect() {
			glDrawBuffer(GL_COLOR_ATTACHMENT0 + _effectTextureAttachment);
		}

		void setReadEffect() {
			glReadBuffer(GL_COLOR_ATTACHMENT0 + _effectTextureAttachment);
		}

		void copyEffectToMainFramebuffer();
		void copyEffectToRenderTarget(TextureRT &target);

		GLuint getTextureID(BufferType type) const {
			return _gTextureIDs[type];
		}

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

		GLuint getFBOID() const {
			return _fboID;
		}

		bool created() const {
			return _fboID != 0;
		}

		GLuint getEffectTextureID() const {
			return _effectTextureID;
		}

		GLuint getDepthTextureID() const {
			return _depthTextureID;
		}
	};
}