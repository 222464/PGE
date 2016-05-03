#pragma once

#include <pge/rendering/bufferobjects/VBO.h>
#include <pge/rendering/shader/Shader.h>

#include <string>

#include <unordered_map>

namespace pge {
	class UBOShaderInterface {
	public:
		struct TextureAndAttachment {
			GLuint _textureHandle, _attachment, _target;

			TextureAndAttachment()
			{}

			TextureAndAttachment(GLuint textureHandle, GLuint attachment, GLuint target)
				: _textureHandle(textureHandle), _attachment(attachment), _target(target)
			{}
		};

	private:
		Shader* _pShader;

		GLuint _blockIndex;
		GLuint _attributeLocation;
		GLuint _bufferBindIndex;

		GLint _blockSize;

	public:
		std::unordered_map<std::string, int> _uniformNameToOffset;

		void create(const std::string &uniformBlockName, Shader* shader, const GLchar** uniformNames, GLsizei numUniformNames);
		void setUpBuffer(VBO &buffer) const;

		void setBindingIndex(GLuint index) {
			_bufferBindIndex = index;

			glUniformBlockBinding(_pShader->getProgramID(), _blockIndex, _bufferBindIndex);
		}

		GLint getBlockSize() const {
			return _blockSize;
		}

		Shader* getShader() const {
			return _pShader;
		}

		void bindBufferToSetIndex(VBO &buffer) {
			glBindBufferBase(GL_UNIFORM_BUFFER, _bufferBindIndex, buffer.getID());
		}

		void unbindSetIndex() {
			glBindBufferBase(GL_UNIFORM_BUFFER, _bufferBindIndex, 0);
		}

		void setUniform(const std::string &name, GLsizei size, GLvoid* param) {
			assert(_uniformNameToOffset.find(name) != _uniformNameToOffset.end());
			glBufferSubData(GL_UNIFORM_BUFFER, _uniformNameToOffset[name], size, param);
		}

		void setUniformi(const std::string &name, GLshort param) {
			assert(_uniformNameToOffset.find(name) != _uniformNameToOffset.end());
			glBufferSubData(GL_UNIFORM_BUFFER, _uniformNameToOffset[name], sizeof(GLshort), &param);
		}

		void setUniformf(const std::string &name, float param) {
			assert(_uniformNameToOffset.find(name) != _uniformNameToOffset.end());
			glBufferSubData(GL_UNIFORM_BUFFER, _uniformNameToOffset[name], sizeof(float), &param);
		}

		void setUniformv2f(const std::string &name, const Vec2f &param) {
			assert(_uniformNameToOffset.find(name) != _uniformNameToOffset.end());
			glBufferSubData(GL_UNIFORM_BUFFER, _uniformNameToOffset[name], sizeof(Vec2f), &param);
		}

		void setUniformv3f(const std::string &name, const Vec3f &param) {
			assert(_uniformNameToOffset.find(name) != _uniformNameToOffset.end());
			glBufferSubData(GL_UNIFORM_BUFFER, _uniformNameToOffset[name], sizeof(Vec3f), &param);
		}

		void setUniformv4f(const std::string &name, const Vec4f &param) {
			assert(_uniformNameToOffset.find(name) != _uniformNameToOffset.end());
			glBufferSubData(GL_UNIFORM_BUFFER, _uniformNameToOffset[name], sizeof(Vec4f), &param);
		}

		void setUniformmat3(const std::string &name, const Matrix3x3f &param) {
			assert(_uniformNameToOffset.find(name) != _uniformNameToOffset.end());

			// Special padding is required for mat3
			std::array<float, 12> data;

			param.getUBOPadded(data);

			glBufferSubData(GL_UNIFORM_BUFFER, _uniformNameToOffset[name], sizeof(float) * 12, &data[0]);
		}

		void setUniformmat4(const std::string &name, const Matrix4x4f &param) {
			assert(_uniformNameToOffset.find(name) != _uniformNameToOffset.end());
			glBufferSubData(GL_UNIFORM_BUFFER, _uniformNameToOffset[name], sizeof(float) * 16, &param._elements[0]);
		}

		void setUniform(int paramLoc, GLsizei size, GLvoid* param) {
			glBufferSubData(GL_UNIFORM_BUFFER, paramLoc, size, param);
		}

		void setUniformf(int paramLoc, float param) {
			glBufferSubData(GL_UNIFORM_BUFFER, paramLoc, sizeof(float), &param);
		}

		void setUniformv2f(int paramLoc, const Vec2f &param) {
			glBufferSubData(GL_UNIFORM_BUFFER, paramLoc, sizeof(Vec2f), &param);
		}

		void setUniformv3f(int paramLoc, const Vec3f &param) {
			glBufferSubData(GL_UNIFORM_BUFFER, paramLoc, sizeof(Vec3f), &param);
		}

		void setUniformv4f(int paramLoc, const Vec4f &param) {
			glBufferSubData(GL_UNIFORM_BUFFER, paramLoc, sizeof(Vec4f), &param);
		}

		void setUniformmat3(int paramLoc, const Matrix3x3f &param) {
			// Special padding is required for mat3
			std::array<float, 12> data;

			param.getUBOPadded(data);

			glBufferSubData(GL_UNIFORM_BUFFER, paramLoc, sizeof(float) * 12, &data[0]);
		}

		void setUniformmat4(int paramLoc, const Matrix4x4f &param) {
			glBufferSubData(GL_UNIFORM_BUFFER, paramLoc, sizeof(float) * 16, &param._elements[0]);
		}
	};
}