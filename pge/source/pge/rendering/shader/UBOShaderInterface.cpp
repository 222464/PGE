#include <pge/rendering/shader/UBOShaderInterface.h>

#include <pge/rendering/material/Material.h>

#include <assert.h>

using namespace pge;

void UBOShaderInterface::create(const std::string &uniformBlockName, Shader* pShader, const GLchar** uniformNames, GLsizei numUniformNames) {
	_pShader = pShader;

	_blockIndex = glGetUniformBlockIndex(_pShader->getProgramID(), uniformBlockName.c_str());

	assert(_blockIndex != GL_INVALID_INDEX);

	glGetActiveUniformBlockiv(_pShader->getProgramID(), _blockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &_blockSize);

#ifdef PGE_DEBUG
	GLint numBlockUniforms;
	glGetActiveUniformBlockiv(_pShader->getProgramID(), _blockIndex, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &numBlockUniforms);
	assert(numBlockUniforms == numUniformNames);
#endif

	std::vector<GLuint> indices(numUniformNames);

	glGetUniformIndices(_pShader->getProgramID(), numUniformNames, uniformNames, &indices[0]);

	// Query offsets and associate names to the offsets
	std::vector<GLint> offsets(numUniformNames);

	glGetActiveUniformsiv(_pShader->getProgramID(), numUniformNames, &indices[0], GL_UNIFORM_OFFSET, &offsets[0]);

	for(size_t i = 0; i < numUniformNames; i++)
		_uniformNameToOffset[uniformNames[i]] = offsets[i];

	_bufferBindIndex = _pShader->_lastUniformBlockBindingIndex++;

	glUniformBlockBinding(_pShader->getProgramID(), _blockIndex, _bufferBindIndex);

	PGE_GL_ERROR_CHECK();
}

void UBOShaderInterface::setUpBuffer(VBO &buffer) const {
	assert(buffer.created());

	buffer.bind(GL_UNIFORM_BUFFER);

	glBufferData(GL_UNIFORM_BUFFER, _blockSize, nullptr, GL_STREAM_DRAW);

	buffer.unbind();
}