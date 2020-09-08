#include "StaticMesh.h"

using namespace pge;

void StaticMesh::create(bool useBuffer) {
	if (useBuffer) {
		interleavedBuffer.create();
		indexBuffer.create();
	}
}

void StaticMesh::updateBuffers() {
	assert(interleavedBuffer.created());
	assert(indexBuffer.created());

	numIndices = indices.size();

	interleavedBuffer.bind(GL_ARRAY_BUFFER);

	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

	indexBuffer.bind(GL_ELEMENT_ARRAY_BUFFER);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(staticMeshIndexType) * indices.size(), &indices[0], GL_STATIC_DRAW);
}

void StaticMesh::renderFromBuffers() {
	interleavedBuffer.bind(GL_ARRAY_BUFFER);

	glVertexAttribPointer(PGE_ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glVertexAttribPointer(PGE_ATTRIB_NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const GLvoid*>(sizeof(Vec3f)));
	glVertexAttribPointer(PGE_ATTRIB_TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const GLvoid*>(sizeof(Vec3f) + sizeof(Vec3f)));

	indexBuffer.bind(GL_ELEMENT_ARRAY_BUFFER);

	glDrawElements(GL_TRIANGLES, numIndices, PGE_STATIC_MESH_INDEX_TYPE_ENUM, nullptr);
}

void StaticMesh::renderFromArrays() {
	VBO::unbind(GL_ARRAY_BUFFER);
	VBO::unbind(GL_ELEMENT_ARRAY_BUFFER);

	glVertexAttribPointer(PGE_ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), &vertices[0].position);
	glVertexAttribPointer(PGE_ATTRIB_NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), &vertices[0].normal);
	glVertexAttribPointer(PGE_ATTRIB_TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), &vertices[0].texCoord);

	glDrawElements(GL_TRIANGLES, indices.size(), PGE_STATIC_MESH_INDEX_TYPE_ENUM, &indices[0]);
}

void StaticMesh::setAttributes() {
	if (hasBuffer()) {
		interleavedBuffer.bind(GL_ARRAY_BUFFER);

		glVertexAttribPointer(PGE_ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
		glVertexAttribPointer(PGE_ATTRIB_NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const GLvoid*>(sizeof(Vec3f)));
		glVertexAttribPointer(PGE_ATTRIB_TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const GLvoid*>(sizeof(Vec3f)+sizeof(Vec3f)));

		indexBuffer.bind(GL_ELEMENT_ARRAY_BUFFER);
	}
	else {
		VBO::unbind(GL_ARRAY_BUFFER);
		VBO::unbind(GL_ELEMENT_ARRAY_BUFFER);

		glVertexAttribPointer(PGE_ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), &vertices[0].position);
		glVertexAttribPointer(PGE_ATTRIB_NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), &vertices[0].normal);
		glVertexAttribPointer(PGE_ATTRIB_TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), &vertices[0].texCoord);
	}
}

void StaticMesh::renderFromAttributes() {
	if (hasBuffer())
		glDrawElements(GL_TRIANGLES, numIndices, PGE_STATIC_MESH_INDEX_TYPE_ENUM, nullptr);
	else
		glDrawElements(GL_TRIANGLES, indices.size(), PGE_STATIC_MESH_INDEX_TYPE_ENUM, &indices[0]);
}