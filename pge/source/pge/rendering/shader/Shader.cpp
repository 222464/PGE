#include "Shader.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>

#include <assert.h>

using namespace pge;

Shader* Shader::pCurrentShader = nullptr;

Shader::Shader()
: progID(0), geomID(0), vertID(0), fragID(0), compID(0),
lastAttachment(0), lastUniformBlockBindingIndex(0)
{}

Shader::~Shader() {
	if (progID != 0) {
		glDeleteProgram(progID);

		if(geomID != 0)
			glDeleteShader(geomID);

		if(vertID != 0)
			glDeleteShader(vertID);

		if(fragID != 0)
			glDeleteShader(fragID);
	}
}

bool Shader::loadShader(const std::string &name, GLuint id) {
	std::ifstream fromFile;

	fromFile.open(name);

	if(!fromFile.is_open()) {
		std::cerr << "Could not open file: " << name << "!" << std::endl;
		return false;
	}

	// Load the shader code into one massive string
	std::string fileString((std::istreambuf_iterator<char>(fromFile)), std::istreambuf_iterator<char>());

	const char* code = fileString.c_str();
	glShaderSource(id, 1, &code, nullptr);
	glCompileShader(id);

	if(checkLog(id)) {
		std::cerr << "- in " << name << std::endl;
		return false;
	}

	return true;
}

bool Shader::loadGeometryShader(const std::string &name, GLuint &id) {
	id = glCreateShader(GL_GEOMETRY_SHADER);

	if(!loadShader(name, id)) {
		glDeleteShader(id);

		return false;
	}

	return true;
}

bool Shader::loadVertexShader(const std::string &name, GLuint &id) {
	id = glCreateShader(GL_VERTEX_SHADER);

	if(!loadShader(name, id)) {
		glDeleteShader(id);

		return false;
	}

	return true;
}

bool Shader::loadFragmentShader(const std::string &name, GLuint &id) {
	id = glCreateShader(GL_FRAGMENT_SHADER);

	if(!loadShader(name, id)) {
		glDeleteShader(id);

		return false;
	}

	return true;
}

bool Shader::loadComputeShader(const std::string &name, GLuint &id) {
	id = glCreateShader(GL_COMPUTE_SHADER);

	if (!loadShader(name, id)) {
		glDeleteShader(id);

		return false;
	}

	return true;
}

bool Shader::checkLog(GLuint id) {
	int result;

	glGetShaderiv(id, GL_COMPILE_STATUS, &result);

	// If did not compile without error, show the log
	if(result == GL_FALSE) {
		int logLength;

		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &logLength);

		if(logLength <= 0) {
			std::cout << "Warning: Invalid log length \"" << logLength << "\": Could not retrieve error log!" << std::endl;

			return false;
		}

		// Allocate the string
		std::string log;

		log.resize(logLength);

		glGetShaderInfoLog(id, logLength, &result, &log[0]);

		// Print the log
		std::cerr << "Unable to compile shader: " << log << std::endl;

		return true;
	}

	return false;
}

bool Shader::link(GLuint id) {
	glLinkProgram(id);

	int result;

	// Check if linking was successful
	glGetProgramiv(id, GL_LINK_STATUS, &result);

	if (result == GL_FALSE) {
		// Not validated, print out the log
		int logLength;

		glGetProgramiv(id, GL_INFO_LOG_LENGTH, &logLength);

		if (logLength <= 0) {
			std::cout << "Warning: Invalid log length \"" << logLength << "\": Could not retrieve error log!" << std::endl;

			return true;
		}

		// Allocate the string
		std::string log;

		log.resize(logLength);

		glGetProgramInfoLog(id, logLength, &result, &log[0]);

		std::cerr << "Unable to link program: " << log << std::endl;

		return false;
	}

	PGE_GL_ERROR_CHECK();

	return true;
}

bool Shader::validate(GLuint id) {
	glValidateProgram(id);

	int result;

	// Check if validation was successful
	glGetProgramiv(id, GL_VALIDATE_STATUS, &result);

	if(result == GL_FALSE) {
		// Not validated, print out the log
		int logLength;
	
		glGetProgramiv(id, GL_INFO_LOG_LENGTH, &logLength);

		if(logLength <= 0) {
			std::cout << "Warning: Invalid log length \"" << logLength << "\": Could not retrieve error log!" << std::endl;

			return true;
		}

		// Allocate the string
		std::string log;

		log.resize(logLength);

		glGetProgramInfoLog(id, logLength, &result, &log[0]);

		std::cerr << "Unable to validate program: " << log << std::endl;

		return false;
	}

	return true;
}

bool Shader::createAsset(const std::string &name) {
	// Name contains vertex shader file name and the fragment shader file name, so extract those
	std::istringstream is(name);

	std::string first;
	is >> first;

	if (first == "COMPUTE") {
		std::string computeName;

		is >> computeName;

		if (!loadComputeShader(computeName, compID)) {
			std::cerr << "Could not load compute shader!" << std::endl;
			return false;
		}
	}
	else {
		std::string geomName, fragName, vertName;
		geomName = first;
		is >> vertName >> fragName;

		// Load the shaders
		if (geomName != "NONE")
		if (!loadGeometryShader(geomName, geomID)) {
			std::cerr << "Could not load geometry shader!" << std::endl;
			return false;
		}

		if (vertName != "NONE")
		if (!loadVertexShader(vertName, vertID)) {
			std::cerr << "Could not load vertex shader!" << std::endl;
			return false;
		}

		if (fragName != "NONE")
		if (!loadFragmentShader(fragName, fragID)) {
			std::cerr << "Could not load fragment shader!" << std::endl;
			return false;
		}
	}

	// Create the main program
	progID = glCreateProgram();

	// Attach the shader components to the program
	if (compID != 0)
		glAttachShader(progID, compID);
	else {
		if (geomID != 0)
			glAttachShader(progID, geomID);

		if (vertID != 0)
			glAttachShader(progID, vertID);

		if (fragID != 0)
			glAttachShader(progID, fragID);
	}

	if (!link(progID)) {
		std::cerr << "- in " << name << std::endl;

		return false;
	}

	PGE_GL_ERROR_CHECK();

	return true;
}

void Shader::setShaderTexture(const std::string &name, GLuint textureID, GLuint target) {
	assert(progID != 0);

	checkProgram();

	// See if the texture already exists, in which case it is simply updated
	std::unordered_map<std::string, TextureAndAttachment>::iterator it = textures.find(name);
		
	// If the texture does not already exist
	if(it == textures.end()) {
		// Check that the uniform exists
		int baseImageLoc = glGetUniformLocation(progID, name.c_str());
			
		if(baseImageLoc != -1) {
			glUniform1i(baseImageLoc, lastAttachment);
				
			// Add the texture to the hash
			textures[name] = TextureAndAttachment(textureID, lastAttachment, target);
				
			lastAttachment++;
		}
	}
	else {
		// Update the existing texture
		it->second.textureHandle = textureID;
		it->second.target = target;
		it->second.isImage = false;
	}
}

void Shader::setShaderTexture(const std::string &name, GLuint textureAttachment, GLuint textureID, GLuint target) {
	assert(progID != 0);

	checkProgram();

	// See if the texture already exists, in which case it is simply updated
	std::unordered_map<std::string, TextureAndAttachment>::iterator it = textures.find(name);
		
	// If the texture does not already exist
	if(it == textures.end()) {
		// Check that the uniform exists
		int baseImageLoc = glGetUniformLocation(progID, name.c_str());
			
		if(baseImageLoc == -1)
			std::cerr << "Could not find the uniform " << name << "!" << std::endl;
		else {
			glUniform1i(baseImageLoc, textureAttachment);
				
			// Add the texture to the hash
			textures[name] = TextureAndAttachment(textureID, textureAttachment, target);
		}
	}
	else {
		// Update the existing texture
		it->second.textureHandle = textureID;
		it->second.attachment = textureAttachment;
		it->second.target = target;
		it->second.isImage = false;
	}
}

void Shader::setShaderImage(const std::string &name, GLuint textureID, GLuint level, bool isLayered, GLint layer, GLenum access, GLenum format) {
	assert(progID != 0);

	checkProgram();

	// See if the texture already exists, in which case it is simply updated
	std::unordered_map<std::string, TextureAndAttachment>::iterator it = textures.find(name);

	// If the texture does not already exist
	if (it == textures.end()) {
		// Check that the uniform exists
		int baseImageLoc = glGetUniformLocation(progID, name.c_str());

		if (baseImageLoc != -1) {
			glUniform1i(baseImageLoc, lastAttachment);

			// Add the texture to the hash
			textures[name] = TextureAndAttachment(textureID, lastAttachment, level, isLayered, layer, access, format);

			lastAttachment++;
		}
	}
	else {
		// Update the existing texture
		it->second.textureHandle = textureID;
		it->second.level = level;
		it->second.isLayered = isLayered;
		it->second.layer = layer;
		it->second.access = access;
		it->second.format = format;
		it->second.isImage = true;
	}
}

void Shader::setShaderImage(const std::string &name, GLuint textureAttachment, GLuint textureID, GLuint level, bool isLayered, GLint layer, GLenum access, GLenum format) {
	assert(progID != 0);

	checkProgram();

	// See if the texture already exists, in which case it is simply updated
	std::unordered_map<std::string, TextureAndAttachment>::iterator it = textures.find(name);

	// If the texture does not already exist
	if (it == textures.end()) {
		// Check that the uniform exists
		int baseImageLoc = glGetUniformLocation(progID, name.c_str());

		if (baseImageLoc == -1)
			std::cerr << "Could not find the uniform " << name << "!" << std::endl;
		else {
			glUniform1i(baseImageLoc, textureAttachment);

			// Add the texture to the hash
			textures[name] = TextureAndAttachment(textureID, textureAttachment, level, isLayered, layer, access, format);
		}
	}
	else {
		// Update the existing texture
		it->second.textureHandle = textureID;
		it->second.attachment = textureAttachment;
		it->second.level = level;
		it->second.isLayered = isLayered;
		it->second.layer = layer;
		it->second.access = access;
		it->second.format = format;
		it->second.isImage = true;
	}
}

void Shader::bindShaderTextures() {
	// Bind all textures
	for(std::unordered_map<std::string, TextureAndAttachment>::iterator it = textures.begin(); it != textures.end(); it++) {
		if (!it->second.isImage) {
			glActiveTexture(GL_TEXTURE0 + it->second.attachment);
			glBindTexture(it->second.target, it->second.textureHandle);
		}
		
	}

	glActiveTexture(GL_TEXTURE0);
}

void Shader::unbindShaderTextures() {
	// Bind all used textures
	for(std::unordered_map<std::string, TextureAndAttachment>::iterator it = textures.begin(); it != textures.end(); it++) {
		if (!it->second.isImage) {
			glActiveTexture(GL_TEXTURE0 + it->second.attachment);
			glBindTexture(it->second.target, 0);
		}
	}

	glActiveTexture(GL_TEXTURE0);
}

int Shader::getAttributeLocation(const std::string &name) {
	int paramLoc;

	std::unordered_map<std::string, int>::iterator it = attributeLocations.find(name);

	if(it == attributeLocations.end())
		attributeLocations[name] = paramLoc = glGetUniformLocation(progID, name.c_str());
	else
		paramLoc = it->second;

	return paramLoc;
}

int Shader::setUniformf(const std::string &name, float param) {
	checkProgram();

	int paramLoc = getAttributeLocation(name);

	if (paramLoc != -1)
		glUniform1f(paramLoc, param);

	return paramLoc;
}

int Shader::setUniformv2f(const std::string &name, float param1, float param2) {
	checkProgram();

	int paramLoc = getAttributeLocation(name);

	if (paramLoc != -1)
		glUniform2f(paramLoc, param1, param2);
	
	return paramLoc;
}

int Shader::setUniformv2f(const std::string &name, const Vec2f &param) {
	return setUniformv2f(name, param.x, param.y);
}

int Shader::setUniformv3f(const std::string &name, float param1, float param2, float param3) {
	checkProgram();

	int paramLoc = getAttributeLocation(name);

	if (paramLoc != -1)
		glUniform3f(paramLoc, param1, param2, param3);
	
	return paramLoc;
}

int Shader::setUniformv3f(const std::string &name, const Vec3f &param) {
	return setUniformv3f(name, param.x, param.y, param.z);
}

int Shader::setUniformv4f(const std::string &name, float param1, float param2, float param3, float param4) {
	checkProgram();

	int paramLoc = getAttributeLocation(name);
	
	if (paramLoc != -1)
		glUniform4f(paramLoc, param1, param2, param3, param4);
	
	return paramLoc;
}

int Shader::setUniformv4f(const std::string &name, const Vec4f &param) {
	return setUniformv4f(name, param.x, param.y, param.z, param.w);
}

int Shader::setUniformi(const std::string &name, int param) {
	checkProgram();

	int paramLoc = getAttributeLocation(name);

	if (paramLoc != -1)
		glUniform1i(paramLoc, param);
	
	return paramLoc;
}

int Shader::setUniformv2i(const std::string &name, int param1, int param2) {
	checkProgram();

	int paramLoc = getAttributeLocation(name);

	if (paramLoc != -1)
		glUniform2i(paramLoc, param1, param2);
	
	return paramLoc;
}

int Shader::setUniformmat3(const std::string &name, const Matrix3x3f &param) {
	checkProgram();

	int paramLoc = getAttributeLocation(name);
	
	if (paramLoc != -1)
		glUniformMatrix3fv(paramLoc, 1, false, &param.elements[0]);
	
	return paramLoc;
}

int Shader::setUniformmat4(const std::string &name, const Matrix4x4f &param) {
	checkProgram();

	int paramLoc = getAttributeLocation(name);
	
	if (paramLoc != -1)
		glUniformMatrix4fv(paramLoc, 1, false, &param.elements[0]);
	
	return paramLoc;
}

int Shader::setTextureAttachmentToUniform(const std::string &name, GLuint attachment) {
	checkProgram();

	int paramLoc = getAttributeLocation(name);
	
	if (paramLoc != -1)
		glUniform1i(paramLoc, attachment);
	
	return paramLoc;
}

void Shader::setTextureAttachmentToUniform(int paramLoc, GLuint attachment) {
	checkProgram();

	if (paramLoc != -1)
		glUniform1i(paramLoc, attachment);
}

int Shader::setUniform1iv(const std::string &name, GLuint numParams, const int* params) {
	checkProgram();

	int paramLoc = getAttributeLocation(name);
	
	if (paramLoc != -1)
		glUniform1iv(paramLoc, numParams, params);
	
	return paramLoc;
}

int Shader::setUniform2iv(const std::string &name, GLuint numParams, const int* params) {
	checkProgram();

	int paramLoc = getAttributeLocation(name);

	if (paramLoc != -1)
		glUniform2iv(paramLoc, numParams, params);
	
	return paramLoc;
}

int Shader::setUniform3iv(const std::string &name, GLuint numParams, const int* params) {
	checkProgram();

	int paramLoc = getAttributeLocation(name);

	if (paramLoc != -1)
		glUniform3iv(paramLoc, numParams, params);
	
	return paramLoc;
}

int Shader::setUniform4iv(const std::string &name, GLuint numParams, const int* params) {
	checkProgram();

	int paramLoc = getAttributeLocation(name);
	
	if (paramLoc != -1)
		glUniform4iv(paramLoc, numParams, params);
	
	return paramLoc;
}

int Shader::setUniform1fv(const std::string &name, GLuint numParams, const float* params) {
	checkProgram();

	int paramLoc = getAttributeLocation(name);
	
	if (paramLoc != -1)
		glUniform1fv(paramLoc, numParams, params);
	
	return paramLoc;
}

int Shader::setUniform2fv(const std::string &name, GLuint numParams, const float* params) {
	checkProgram();

	int paramLoc = getAttributeLocation(name);

	if (paramLoc != -1)
		glUniform2fv(paramLoc, numParams, params);
	
	return paramLoc;
}

int Shader::setUniform3fv(const std::string &name, GLuint numParams, const float* params) {
	checkProgram();

	int paramLoc = getAttributeLocation(name);

	if (paramLoc != -1)
		glUniform3fv(paramLoc, numParams, params);
	
	return paramLoc;
}

int Shader::setUniform4fv(const std::string &name, GLuint numParams, const float* params) {
	checkProgram();

	int paramLoc = getAttributeLocation(name);

	if (paramLoc != -1)
		glUniform4fv(paramLoc, numParams, params);

	return paramLoc;
}

// -------------------- Attribute Location Versions ----------------------

void Shader::setUniformf(int paramLoc, float param) {
	checkProgram();
	
	if (paramLoc != -1)
		glUniform1f(paramLoc, param);
}

void Shader::setUniformv2f(int paramLoc, float param1, float param2) {
	checkProgram();

	if (paramLoc != -1)
		glUniform2f(paramLoc, param1, param2);
}

void Shader::setUniformv2f(int paramLoc, const Vec2f &param) {
	setUniformv2f(paramLoc, param.x, param.y);
}

void Shader::setUniformv3f(int paramLoc, float param1, float param2, float param3) {
	checkProgram();
	
	if (paramLoc != -1)
		glUniform3f(paramLoc, param1, param2, param3);
}

void Shader::setUniformv3f(int paramLoc, const Vec3f &param) {
	setUniformv3f(paramLoc, param.x, param.y, param.z);
}

void Shader::setUniformv4f(int paramLoc, float param1, float param2, float param3, float param4) {
	checkProgram();

	if (paramLoc != -1)
		glUniform4f(paramLoc, param1, param2, param3, param4);
}

void Shader::setUniformv4f(int paramLoc, const Vec4f &param) {
	setUniformv4f(paramLoc, param.x, param.y, param.z, param.w);
}

void Shader::setUniformi(int paramLoc, int param) {
	checkProgram();

	if (paramLoc != -1)
		glUniform1i(paramLoc, param);
}

void Shader::setUniformv2i(int paramLoc, int param1, int param2) {
	checkProgram();
	
	if (paramLoc != -1)
		glUniform2i(paramLoc, param1, param2);
}

void Shader::setUniformmat3(int paramLoc, const Matrix3x3f &param) {
	checkProgram();

	if (paramLoc != -1)
		glUniformMatrix3fv(paramLoc, 1, false, &param.elements[0]);
}

void Shader::setUniformmat4(int paramLoc, const Matrix4x4f &param) {
	checkProgram();
	
	if (paramLoc != -1)
		glUniformMatrix4fv(paramLoc, 1, false, &param.elements[0]);
}

void Shader::setUniform1iv(int paramLoc, GLuint numParams, const int* params) {
	checkProgram();

	if (paramLoc != -1)
		glUniform1iv(paramLoc, numParams, params);
}

void Shader::setUniform2iv(int paramLoc, GLuint numParams, const int* params) {
	checkProgram();

	if (paramLoc != -1)
		glUniform2iv(paramLoc, numParams, params);
}

void Shader::setUniform3iv(int paramLoc, GLuint numParams, const int* params) {
	checkProgram();

	if (paramLoc != -1)
		glUniform3iv(paramLoc, numParams, params);
}

void Shader::setUniform4iv(int paramLoc, GLuint numParams, const int* params) {
	checkProgram();
	
	if (paramLoc != -1)
		glUniform4iv(paramLoc, numParams, params);
}

void Shader::setUniform1fv(int paramLoc, GLuint numParams, const float* params) {
	checkProgram();

	if (paramLoc != -1)
		glUniform1fv(paramLoc, numParams, params);
}

void Shader::setUniform2fv(int paramLoc, GLuint numParams, const float* params) {
	checkProgram();
	
	if (paramLoc != -1)
		glUniform2fv(paramLoc, numParams, params);
}

void Shader::setUniform3fv(int paramLoc, GLuint numParams, const float* params) {
	checkProgram();

	if (paramLoc != -1)
		glUniform3fv(paramLoc, numParams, params);
}

void Shader::setUniform4fv(int paramLoc, GLuint numParams, const float* params) {
	checkProgram();
	
	if (paramLoc != -1)
		glUniform4fv(paramLoc, numParams, params);
}
