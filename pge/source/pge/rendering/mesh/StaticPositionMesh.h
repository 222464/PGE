#pragma once

#include <pge/constructs/Vec3f.h>
#include <pge/constructs/Vec2f.h>

#include <pge/rendering/SFMLOGL.h>
#include <pge/rendering/bufferobjects/VBO.h>

#define D3D_STATIC_POSITION_MESH_INDEX_TYPE_ENUM GL_UNSIGNED_SHORT

namespace pge {
	typedef GLushort staticPositionMeshIndexType;

	class StaticPositionMesh {
	private:
		VBO _positionBuffer;
		VBO _indexBuffer;

	public:
		std::vector<Vec3f> _vertices;
		std::vector<staticPositionMeshIndexType> _indices;

		size_t _numIndices;

		StaticPositionMesh()
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
			return _positionBuffer.created();
		}
	};
}