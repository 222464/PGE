#include "TerrainGrass.h"

#include "TerrainGrassBatcher.h"

#include "../../util/Math.h"

using namespace pge;

void TerrainGrass::create(const std::vector<Vec3f> &grassPositions, const Vec3f &chunkCorner,
    const SceneObjectRef &grassBatchRenderer, int tilesInX, int tilesInY, int numTiles, float maxTopSkew)
{
    renderMask = 0xffff;

    this->chunkCorner = chunkCorner;
    this->grassBatchRenderer = grassBatchRenderer;

    staticMeshes.push_back(std::shared_ptr<StaticMesh>(new StaticMesh()));

    staticMeshes.back()->create(true);
    staticMeshes.back()->indices.reserve(grassPositions.size() * 12);
    staticMeshes.back()->vertices.reserve(grassPositions.size() * 8);

    std::uniform_real_distribution<float> distRad(0.0f, piTimes2);

    const Vec3f quadPositions[8] {
        Vec3f(-0.5f, 0.0f, 0.0f),
        Vec3f(0.5f, 0.0f, 0.0f),
        Vec3f(0.5f, 1.0f, 0.0f),
        Vec3f(-0.5f, 1.0f, 0.0f),
        Vec3f(-0.5f, 0.0f, 0.0f),
        Vec3f(0.5f, 0.0f, 0.0f),
        Vec3f(0.5f, 1.0f, 0.0f),
        Vec3f(-0.5f, 1.0f, 0.0f)
    };

    const staticMeshIndexType quadIndices[12] {
        0, 1, 2, 0, 2, 3, 5, 4, 7, 5, 7, 6
    };

    const Vec2f quadTexCoords[8] {
        Vec2f(0.0f, 0.0f),
        Vec2f(1.0f, 0.0f),
        Vec2f(1.0f, 1.0f),
        Vec2f(0.0f, 1.0f),
        Vec2f(0.0f, 0.0f),
        Vec2f(1.0f, 0.0f),
        Vec2f(1.0f, 1.0f),
        Vec2f(0.0f, 1.0f)
    };

    float subsurfaceScatteringFactor = 0.5f;

    const Vec3f quadNormals[8] {
        Vec3f(0.0f, 0.0f, -subsurfaceScatteringFactor),
        Vec3f(0.0f, 0.0f, -subsurfaceScatteringFactor),
        Vec3f(0.0f, 0.0f, -subsurfaceScatteringFactor),
        Vec3f(0.0f, 0.0f, -subsurfaceScatteringFactor),
        Vec3f(0.0f, 0.0f, subsurfaceScatteringFactor),
        Vec3f(0.0f, 0.0f, subsurfaceScatteringFactor),
        Vec3f(0.0f, 0.0f, subsurfaceScatteringFactor),
        Vec3f(0.0f, 0.0f, subsurfaceScatteringFactor)
    };

    aabb.lowerBound = aabb.upperBound = chunkCorner;

    std::uniform_int_distribution<int> distTile(0, numTiles - 1);
    std::uniform_real_distribution<float> dist11(-1.0f, 1.0f);
    std::uniform_real_distribution<float> dist01(0.0f, 1.0f);

    float tInXInv = 1.0f / tilesInX;
    float tInYInv = 1.0f / tilesInY;

    size_t grassCounter = 0;

    for (size_t i = 0; i < grassPositions.size(); i++, grassCounter++) {
        if (staticMeshes.back()->indices.size() >= std::numeric_limits<staticMeshIndexType>().max()) {
            // New mesh
            staticMeshes.push_back(std::shared_ptr<StaticMesh>(new StaticMesh()));

            staticMeshes.back()->create(true);

            grassCounter = 0;
        }

        float angle = distRad(getScene()->randomGenerator);
        Matrix4x4f transform(Matrix4x4f::translateMatrix(grassPositions[i]) * Matrix4x4f::rotateMatrixY(angle));

        size_t vertexIndex = grassCounter * 8;

        int tileIndex = distTile(getScene()->randomGenerator);

        float tx = static_cast<float>(tileIndex % tilesInX);
        float ty = static_cast<float>(tileIndex / tilesInX);

        Vec3f skewVec(dist01(getScene()->randomGenerator) * maxTopSkew * Vec3f(dist11(getScene()->randomGenerator), 0.0f, dist11(getScene()->randomGenerator)).normalized());

        for (size_t j = 0; j < 8; j++) {
            staticMeshes.back()->vertices.push_back(pge::StaticMesh::Vertex());

            staticMeshes.back()->vertices.back().position = transform * quadPositions[j];

            aabb.expand(staticMeshes.back()->vertices.back().position);

            staticMeshes.back()->vertices.back().normal = Quaternion(angle, Vec3f(0.0f, 1.0f, 0.0f)) * quadNormals[j];

            staticMeshes.back()->vertices.back().texCoord = Vec2f((tx + quadTexCoords[j].x) * tInXInv, (ty + quadTexCoords[j].y) * tInYInv);

            // Apply skew
            switch (j) {
            case 2:
            case 3:
            case 6:
            case 7:
                staticMeshes.back()->vertices.back().position += skewVec;

                break;
            }
        }

        for (size_t j = 0; j < 12; j++)
            staticMeshes.back()->indices.push_back(vertexIndex + quadIndices[j]);
    }

    for (size_t i = 0; i < staticMeshes.size(); i++)
        staticMeshes[i]->updateBuffers();

    aabb.calculateHalfDims();
    aabb.calculateCenter();

    updateAABB();
}

void TerrainGrass::deferredRender() {
    assert(grassBatchRenderer.isAlive());

    static_cast<TerrainGrassBatcher*>(grassBatchRenderer.get())->grassObjects.push_back(*this);
}