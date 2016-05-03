#include <pge/rendering/mesh/StaticMesh.h>

using namespace pge;

void StaticMesh::create(bool useBuffer) {
	if (useBuffer) {
		_interleavedBuffer.create();
		_indexBuffer.create();
	}
}

void StaticMesh::updateBuffers() {
	assert(_interleavedBuffer.created());
	assert(_indexBuffer.created());

	_numIndices = _indices.size();

	_interleavedBuffer.bind(GL_ARRAY_BUFFER);

	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * _vertices.size(), &_vertices[0], GL_STATIC_DRAW);

	_indexBuffer.bind(GL_ELEMENT_ARRAY_BUFFER);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(staticMeshIndexType) * _indices.size(), &_indices[0], GL_STATIC_DRAW);
}

void StaticMesh::renderFromBuffers() {
	_interleavedBuffer.bind(GL_ARRAY_BUFFER);

	glVertexAttribPointer(PGE_ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glVertexAttribPointer(PGE_ATTRIB_NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const GLvoid*>(sizeof(Vec3f)));
	glVertexAttribPointer(PGE_ATTRIB_TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const GLvoid*>(sizeof(Vec3f) + sizeof(Vec3f)));

	_indexBuffer.bind(GL_ELEMENT_ARRAY_BUFFER);

	glDrawElements(GL_TRIANGLES, _numIndices, PGE_STATIC_MESH_INDEX_TYPE_ENUM, nullptr);
}

void StaticMesh::renderFromArrays() {
	VBO::unbind(GL_ARRAY_BUFFER);
	VBO::unbind(GL_ELEMENT_ARRAY_BUFFER);

	glVertexAttribPointer(PGE_ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), &_vertices[0]._position);
	glVertexAttribPointer(PGE_ATTRIB_NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), &_vertices[0]._normal);
	glVertexAttribPointer(PGE_ATTRIB_TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), &_vertices[0]._texCoord);

	glDrawElements(GL_TRIANGLES, _indices.size(), PGE_STATIC_MESH_INDEX_TYPE_ENUM, &_indices[0]);
}

void StaticMesh::setAttributes() {
	if (hasBuffer()) {
		_interleavedBuffer.bind(GL_ARRAY_BUFFER);

		glVertexAttribPointer(PGE_ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
		glVertexAttribPointer(PGE_ATTRIB_NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const GLvoid*>(sizeof(Vec3f)));
		glVertexAttribPointer(PGE_ATTRIB_TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const GLvoid*>(sizeof(Vec3f)+sizeof(Vec3f)));

		_indexBuffer.bind(GL_ELEMENT_ARRAY_BUFFER);
	}
	else {
		VBO::unbind(GL_ARRAY_BUFFER);
		VBO::unbind(GL_ELEMENT_ARRAY_BUFFER);

		glVertexAttribPointer(PGE_ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), &_vertices[0]._position);
		glVertexAttribPointer(PGE_ATTRIB_NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), &_vertices[0]._normal);
		glVertexAttribPointer(PGE_ATTRIB_TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), &_vertices[0]._texCoord);
	}
}

void StaticMesh::renderFromAttributes() {
	if (hasBuffer())
		glDrawElements(GL_TRIANGLES, _numIndices, PGE_STATIC_MESH_INDEX_TYPE_ENUM, nullptr);
	else
		glDrawElements(GL_TRIANGLES, _indices.size(), PGE_STATIC_MESH_INDEX_TYPE_ENUM, &_indices[0]);
}