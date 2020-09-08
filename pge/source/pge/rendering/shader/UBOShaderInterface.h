#pragma once

#include "../bufferobjects/VBO.h"
#include "Shader.h"

#include <string>

#include <unordered_map>

namespace pge {
	class UBOShaderInterface {
	public:
		struct TextureAndAttachment {
			GLuint textureHandle, attachment, target;

			TextureAndAttachment()
			{}

			TextureAndAttachment(GLuint textureHandle, GLuint attachment, GLuint target)
				: textureHandle(textureHandle), attachment(attachment), target(target)
			{}
		};

	private:
		Shader* pShader;

		GLuint blockIndex;
		GLuint attributeLocation;
		GLuint bufferBindIndex;

		GLint blockSize;

	public:
		std::unordered_map<std::string, int> uniformNameToOffset;

		void create(const std::string &uniformBlockName, Shader* shader, const GLchar** uniformNames, GLsizei numUniformNames);
		void setUpBuffer(VBO &buffer) const;

		void setBindingIndex(GLuint index) {
			bufferBindIndex = index;

			glUniformBlockBinding(pShader->getProgramID(), blockIndex, bufferBindIndex);
		}

		GLint getBlockSize() const {
			return blockSize;
		}

		Shader* getShader() const {
			return pShader;
		}

		void bindBufferToSetIndex(VBO &buffer) {
			glBindBufferBase(GL_UNIFORM_BUFFER, bufferBindIndex, buffer.getID());
		}

		void unbindSetIndex() {
			glBindBufferBase(GL_UNIFORM_BUFFER, bufferBindIndex, 0);
		}

		void setUniform(const std::string &name, GLsizei size, GLvoid* param) {
			assert(uniformNameToOffset.find(name) != uniformNameToOffset.end());
			glBufferSubData(GL_UNIFORM_BUFFER, uniformNameToOffset[name], size, param);
		}

		void setUniformi(const std::string &name, GLshort param) {
			assert(uniformNameToOffset.find(name) != uniformNameToOffset.end());
			glBufferSubData(GL_UNIFORM_BUFFER, uniformNameToOffset[name], sizeof(GLshort), &param);
		}

		void setUniformf(const std::string &name, float param) {
			assert(uniformNameToOffset.find(name) != uniformNameToOffset.end());
			glBufferSubData(GL_UNIFORM_BUFFER, uniformNameToOffset[name], sizeof(float), &param);
		}

		void setUniformv2f(const std::string &name, const Vec2f &param) {
			assert(uniformNameToOffset.find(name) != uniformNameToOffset.end());
			glBufferSubData(GL_UNIFORM_BUFFER, uniformNameToOffset[name], sizeof(Vec2f), &param);
		}

		void setUniformv3f(const std::string &name, const Vec3f &param) {
			assert(uniformNameToOffset.find(name) != uniformNameToOffset.end());
			glBufferSubData(GL_UNIFORM_BUFFER, uniformNameToOffset[name], sizeof(Vec3f), &param);
		}

		void setUniformv4f(const std::string &name, const Vec4f &param) {
			assert(uniformNameToOffset.find(name) != uniformNameToOffset.end());
			glBufferSubData(GL_UNIFORM_BUFFER, uniformNameToOffset[name], sizeof(Vec4f), &param);
		}

		void setUniformmat3(const std::string &name, const Matrix3x3f &param) {
			assert(uniformNameToOffset.find(name) != uniformNameToOffset.end());

			// Special padding is required for mat3
			std::array<float, 12> data;

			param.getUBOPadded(data);

			glBufferSubData(GL_UNIFORM_BUFFER, uniformNameToOffset[name], sizeof(float) * 12, &data[0]);
		}

		void setUniformmat4(const std::string &name, const Matrix4x4f &param) {
			assert(uniformNameToOffset.find(name) != uniformNameToOffset.end());
			glBufferSubData(GL_UNIFORM_BUFFER, uniformNameToOffset[name], sizeof(float) * 16, &param.elements[0]);
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
			glBufferSubData(GL_UNIFORM_BUFFER, paramLoc, sizeof(float) * 16, &param.elements[0]);
		}
	};
}