#pragma once

#include "../../constructs/Vec3f.h"
#include "../../constructs/Vec2f.h"

#include "../SFMLOGL.h"
#include "../bufferobjects/VBO.h"

#define PGE_STATIC_POSITION_MESH_INDEX_TYPE_ENUM GL_UNSIGNED_INT

namespace pge {
    typedef GLuint staticPositionMeshIndexType;

    class StaticPositionMesh {
    private:
        VBO positionBuffer;
        VBO indexBuffer;

    public:
        std::vector<Vec3f> vertices;
        std::vector<staticPositionMeshIndexType> indices;

        size_t numIndices;

        StaticPositionMesh()
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
            return positionBuffer.created();
        }
    };
}
