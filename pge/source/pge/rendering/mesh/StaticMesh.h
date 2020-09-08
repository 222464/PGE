#pragma once

#include "../../constructs/Vec3f.h"
#include "../../constructs/Vec2f.h"

#include "../SFMLOGL.h"
#include "../bufferobjects/VBO.h"
#include "../material/Material.h"

#define PGE_STATIC_MESH_INDEX_TYPE_ENUM GL_UNSIGNED_SHORT

namespace pge {
	typedef GLushort staticMeshIndexType;

	class StaticMesh {
	public:
		struct Vertex {
			Vec3f position;
			Vec3f normal;
			Vec2f texCoord;
		};
	private:
		VBO interleavedBuffer;
		VBO indexBuffer;

	public:
		std::vector<Vertex> vertices;
		std::vector<staticMeshIndexType> indices;

		size_t numIndices;

		StaticMesh()
			: numIndices(0)
		{}

		void create(bool useBuffer = true);

		void updateBuffers();

		void clearArrays() {
			vertices.clear();
			indices.clear();
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
			return interleavedBuffer.created();
		}
	};
}