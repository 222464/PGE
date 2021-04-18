#include "StaticPositionMesh.h"

using namespace pge;

void StaticPositionMesh::create(bool useBuffer) {
    if (useBuffer) {
        positionBuffer.create();
        indexBuffer.create();
    }
}

void StaticPositionMesh::updateBuffers() {
    assert(positionBuffer.created());
    assert(indexBuffer.created());

    numIndices = indices.size();

    positionBuffer.bind(GL_ARRAY_BUFFER);

    glBufferData(GL_ARRAY_BUFFER, sizeof(Vec3f) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

    indexBuffer.bind(GL_ELEMENT_ARRAY_BUFFER);

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(staticPositionMeshIndexType) * indices.size(), &indices[0], GL_STATIC_DRAW);
}

void StaticPositionMesh::renderFromBuffers() {
    positionBuffer.bind(GL_ARRAY_BUFFER);

    glVertexAttribPointer(PGE_ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(Vec3f), 0);

    indexBuffer.bind(GL_ELEMENT_ARRAY_BUFFER);

        glEnableVertexAttribArray(PGE_ATTRIB_POSITION);
    glDisableVertexAttribArray(PGE_ATTRIB_NORMAL);
    glDisableVertexAttribArray(PGE_ATTRIB_TEXCOORD);

    glDrawElements(GL_TRIANGLES, numIndices, PGE_STATIC_POSITION_MESH_INDEX_TYPE_ENUM, nullptr);
}

void StaticPositionMesh::renderFromArrays() {
    glDisableVertexAttribArray(PGE_ATTRIB_NORMAL);
    glDisableVertexAttribArray(PGE_ATTRIB_TEXCOORD);

    glVertexAttribPointer(PGE_ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(Vec3f), &vertices[0]);

    VBO::unbind(GL_ARRAY_BUFFER);
    VBO::unbind(GL_ELEMENT_ARRAY_BUFFER);

    glDrawElements(GL_TRIANGLES, indices.size(), PGE_STATIC_POSITION_MESH_INDEX_TYPE_ENUM, &indices[0]);

    glEnableVertexAttribArray(PGE_ATTRIB_NORMAL);
    glEnableVertexAttribArray(PGE_ATTRIB_TEXCOORD);
}

void StaticPositionMesh::setAttributes() {
    if (hasBuffer()) {
        positionBuffer.bind(GL_ARRAY_BUFFER);

                glEnableVertexAttribArray(PGE_ATTRIB_POSITION);
                glDisableVertexAttribArray(PGE_ATTRIB_NORMAL);
                glDisableVertexAttribArray(PGE_ATTRIB_TEXCOORD);

        glVertexAttribPointer(PGE_ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(Vec3f), 0);

        indexBuffer.bind(GL_ELEMENT_ARRAY_BUFFER);
    }
    else {
        VBO::unbind(GL_ARRAY_BUFFER);
        VBO::unbind(GL_ELEMENT_ARRAY_BUFFER);

        glVertexAttribPointer(PGE_ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(Vec3f), &vertices[0]);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
}

void StaticPositionMesh::renderFromAttributes() {
    if (hasBuffer())
        glDrawElements(GL_TRIANGLES, numIndices, PGE_STATIC_POSITION_MESH_INDEX_TYPE_ENUM, nullptr);
    else
        glDrawElements(GL_TRIANGLES, indices.size(), PGE_STATIC_POSITION_MESH_INDEX_TYPE_ENUM, &indices[0]);
}
