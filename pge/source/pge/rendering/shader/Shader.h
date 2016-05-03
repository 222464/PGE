#pragma once

#include <pge/assetManager/Asset.h>

#include <pge/rendering/SFMLOGL.h>

#include <pge/constructs/Matrix3x3f.h>
#include <pge/constructs/Matrix4x4f.h>
#include <pge/constructs/Vec2f.h>
#include <pge/constructs/Vec3f.h>
#include <pge/constructs/Vec4f.h>

#include <string>
#include <unordered_map>

namespace pge {
	class Shader : public Asset {
	public:
		struct TextureAndAttachment {
			// Common
			GLuint _textureHandle, _attachment;

			bool _isImage;

			// Texture only stuff
			GLuint _target;

			// Image only stuff
			GLuint _level;
			bool _isLayered;
			GLint _layer;
			GLenum _access;
			GLenum _format;

			TextureAndAttachment()
			{}

			TextureAndAttachment(GLuint textureHandle, GLuint attachment, GLuint target)
				: _textureHandle(textureHandle), _attachment(attachment),
				_isImage(false),
				_target(target)
			{}

			TextureAndAttachment(GLuint textureHandle, GLuint attachment, GLuint level, bool isLayered, GLint layer, GLenum access, GLenum format)
				: _textureHandle(textureHandle), _attachment(attachment),
				_isImage(true), 
				_level(level), _isLayered(isLayered), _layer(layer), _access(access), _format(format)
			{}
		};

	private:
		GLuint _progID;
		GLuint _geomID, _vertID, _fragID, _compID;

		std::unordered_map<std::string, TextureAndAttachment> _textures;

		// Cache attribute locations for speed
		std::unordered_map<std::string, int> _attributeLocations;

		GLuint _lastAttachment;

		void checkProgram() {
#ifdef PGE_DEBUG
			GLint programID;
			glGetIntegerv(GL_CURRENT_PROGRAM, &programID);

			if (programID != _progID) {
				std::cerr << "Attempted to set shader parameter before binding!" << std::endl;
				abort();
			}
#endif
		}

		GLuint _lastUniformBlockBindingIndex;

		static Shader* _pCurrentShader;

	public:
		static bool loadShader(const std::string &name, GLuint id);
		static bool loadGeometryShader(const std::string &name, GLuint &id);
		static bool loadVertexShader(const std::string &name, GLuint &id);
		static bool loadFragmentShader(const std::string &name, GLuint &id);
		static bool loadComputeShader(const std::string &name, GLuint &id);

		// Returns true if there were compilation errors
		static bool checkLog(GLuint id);

		static bool link(GLuint id);

		static bool validate(GLuint id);

		Shader();
		~Shader();

		// Name contains both shader file names, in format: "geomName vertName fragName". "NONE" can be specified to indicate no shader
		// Inherited from Asset
		bool createAsset(const std::string &name);

		void setShaderTexture(const std::string &name, GLuint textureID, GLuint target);
		void setShaderTexture(const std::string &name, GLuint textureAttachment, GLuint textureID, GLuint target);

		void setShaderImage(const std::string &name, GLuint textureID, GLuint level, bool isLayered, GLint layer, GLenum access, GLenum format);
		void setShaderImage(const std::string &name, GLuint textureAttachment, GLuint textureID, GLuint level, bool isLayered, GLint layer, GLenum access, GLenum format);

		int getAttributeLocation(const std::string &name);

		// ---------------------------- Using Parameter Name ----------------------------

		int setUniformf(const std::string &name, float param);
		int setUniformv2f(const std::string &name, float param1, float param2);
		int setUniformv2f(const std::string &name, const Vec2f &param);
		int setUniformv3f(const std::string &name, float param1, float param2, float param3);
		int setUniformv3f(const std::string &name, const Vec3f &param);
		int setUniformv4f(const std::string &name, float param1, float param2, float param3, float param4);
		int setUniformv4f(const std::string &name, const Vec4f &param);
		int setUniformi(const std::string &name, int param);
		int setUniformv2i(const std::string &name, int param1, int param2);
		int setUniformmat3(const std::string &name, const Matrix3x3f &param);
		int setUniformmat4(const std::string &name, const Matrix4x4f &param);

		// Array uniforms
		int setUniform1iv(const std::string &name, GLuint numParams, const int* params);
		int setUniform2iv(const std::string &name, GLuint numParams, const int* params);
		int setUniform3iv(const std::string &name, GLuint numParams, const int* params);
		int setUniform4iv(const std::string &name, GLuint numParams, const int* params);
		int setUniform1fv(const std::string &name, GLuint numParams, const float* params);
		int setUniform2fv(const std::string &name, GLuint numParams, const float* params);
		int setUniform3fv(const std::string &name, GLuint numParams, const float* params);
		int setUniform4fv(const std::string &name, GLuint numParams, const float* params);

		// ---------------------------- Using Attribute Location ----------------------------

		void setUniformf(int paramLoc, float param);
		void setUniformv2f(int paramLoc, float param1, float param2);
		void setUniformv2f(int paramLoc, const Vec2f &param);
		void setUniformv3f(int paramLoc, float param1, float param2, float param3);
		void setUniformv3f(int paramLoc, const Vec3f &param);
		void setUniformv4f(int paramLoc, float param1, float param2, float param3, float param4);
		void setUniformv4f(int paramLoc, const Vec4f &param);
		void setUniformi(int paramLoc, int param);
		void setUniformv2i(int paramLoc, int param1, int param2);
		void setUniformmat3(int paramLoc, const Matrix3x3f &param);
		void setUniformmat4(int paramLoc, const Matrix4x4f &param);

		// Array uniforms
		void setUniform1iv(int paramLoc, GLuint numParams, const int* params);
		void setUniform2iv(int paramLoc, GLuint numParams, const int* params);
		void setUniform3iv(int paramLoc, GLuint numParams, const int* params);
		void setUniform4iv(int paramLoc, GLuint numParams, const int* params);
		void setUniform1fv(int paramLoc, GLuint numParams, const float* params);
		void setUniform2fv(int paramLoc, GLuint numParams, const float* params);
		void setUniform3fv(int paramLoc, GLuint numParams, const float* params);
		void setUniform4fv(int paramLoc, GLuint numParams, const float* params);

		void bind() {
			if (_pCurrentShader != this) {
				glUseProgram(_progID);

				_pCurrentShader = this;
			}
		}

		void forceBind() {
			glUseProgram(_progID);

			_pCurrentShader = this;
		}

		static void unbind() {
			glUseProgram(0);

			_pCurrentShader = nullptr;
		}

		GLuint getProgramID() const {
			return _progID;
		}

		static Shader* getCurrentShader() {
			return _pCurrentShader;
		}

		// Manual texture unit setting
		int setTextureAttachmentToUniform(const std::string &name, GLuint attachment);
		void setTextureAttachmentToUniform(int paramLoc, GLuint attachment);
		void bindShaderTextures();
		void unbindShaderTextures();

		void bindShaderImages();
		void unbindShaderImages();

		static Asset* assetFactory() {
			return new Shader();
		}

		friend class UBOShaderInterface;
	};
}