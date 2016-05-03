#pragma once

#include <pge/constructs/Vec3f.h>
#include <pge/constructs/Vec2f.h>

#include <pge/rendering/SFMLOGL.h>
#include <pge/rendering/bufferobjects/VBO.h>
#include <pge/rendering/material/Material.h>

#define PGE_STATIC_MESH_INDEX_TYPE_ENUM GL_UNSIGNED_SHORT

namespace pge {
	typedef GLushort staticMeshIndexType;

	class StaticMesh {
	public:
		struct Vertex {
			Vec3f _position;
			Vec3f _normal;
			Vec2f _texCoord;
		};
	private:
		VBO _interleavedBuffer;
		VBO _indexBuffer;

	public:
		std::vector<Vertex> _vertices;
		std::vector<staticMeshIndexType> _indices;

		size_t _numIndices;

		StaticMesh()
			: _numIndices(0)
		{}

		void create(bool useBuffer = true);

		void updateBuffers();

		void clearArrays() {
			_vertices.clear();
			_indices.clear();
		}

		void renderFromBuffers();
		void renderFromArrays();

		void setAttributes();
		void renderFromAttributes();

		void render() {
			if (hasBuffer())
				renderFromBuffers();
			else
				renderFromArrays();
		}

		bool hasBuffer() const {
			return _interleavedBuffer.created();
		}
	};
}