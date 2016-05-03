#pragma once

#include <pge/rendering/SFMLOGL.h>

#include <pge/constructs/Quaternion.h>

#include <pge/rendering/Camera.h>

#include <assert.h>

namespace pge {
	class CubeMapDepthRT {
	public:
		enum Precision {
			_16, _32
		};
	private:
		GLuint _fboID;
		GLuint _depthCubeMapID;

		unsigned int _resolution;

		std::array<Quaternion, 6> _baseViewRotations;

	public:
		CubeMapDepthRT()
			: _fboID(0)
		{}

		~CubeMapDepthRT() {
			if (created())
				destroy();
		}

		void create(unsigned int resolution, Precision precision = _16);
		void destroy();

		void renderFace(class RenderScene* pRenderScene, const Vec3f &position, unsigned char face, float zNear, float zFar, float distance);

		unsigned int getResolution() const {
			return _resolution;
		}

		void setViewport() {
			glViewport(0, 0, _resolution, _resolution);
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
			return _depthCubeMapID;
		}

		GLuint getFBOID() const {
			return _fboID;
		}

		bool created() const {
			return _fboID != 0;
		}

		Camera getCamera(unsigned char face, const Vec3f &position, float zNear, float zFar) const;
	};
}