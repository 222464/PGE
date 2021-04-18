#include "UBOShaderInterface.h"

#include "../material/Material.h"

#include <assert.h>

using namespace pge;

void UBOShaderInterface::create(const std::string &uniformBlockName, Shader* pShader, const GLchar** uniformNames, GLsizei numUniformNames) {
    this->pShader = pShader;

    blockIndex = glGetUniformBlockIndex(pShader->getProgramID(), uniformBlockName.c_str());

    assert(blockIndex != GL_INVALID_INDEX);

    glGetActiveUniformBlockiv(pShader->getProgramID(), blockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);

#ifdef PGE_DEBUG
    GLint numBlockUniforms;
    glGetActiveUniformBlockiv(pShader->getProgramID(), blockIndex, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &numBlockUniforms);
    assert(numBlockUniforms == numUniformNames);
#endif

    std::vector<GLuint> indices(numUniformNames);

    glGetUniformIndices(pShader->getProgramID(), numUniformNames, uniformNames, &indices[0]);

    // Query offsets and associate names to the offsets
    std::vector<GLint> offsets(numUniformNames);

    glGetActiveUniformsiv(pShader->getProgramID(), numUniformNames, &indices[0], GL_UNIFORM_OFFSET, &offsets[0]);

    for(size_t i = 0; i < numUniformNames; i++)
        uniformNameToOffset[uniformNames[i]] = offsets[i];

    bufferBindIndex = pShader->lastUniformBlockBindingIndex++;

    glUniformBlockBinding(pShader->getProgramID(), blockIndex, bufferBindIndex);

    PGE_GL_ERROR_CHECK();
}

void UBOShaderInterface::setUpBuffer(VBO &buffer) const {
    assert(buffer.created());

    buffer.bind(GL_UNIFORM_BUFFER);

    glBufferData(GL_UNIFORM_BUFFER, blockSize, nullptr, GL_STREAM_DRAW);

    buffer.unbind();
}