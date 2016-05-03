#include <pge/rendering/voxel/TerrainGrass.h>

#include <pge/rendering/voxel/TerrainGrassBatcher.h>

#include <pge/util/Math.h>

using namespace pge;

void TerrainGrass::create(const std::vector<Vec3f> &grassPositions, const Vec3f &chunkCorner,
	const SceneObjectRef &grassBatchRenderer, int tilesInX, int tilesInY, int numTiles, float maxTopSkew)
{
	_renderMask = 0xffff;

	_chunkCorner = chunkCorner;
	_grassBatchRenderer = grassBatchRenderer;

	_staticMeshes.push_back(std::shared_ptr<StaticMesh>(new StaticMesh()));

	_staticMeshes.back()->create(true);
	_staticMeshes.back()->_indices.reserve(grassPositions.size() * 12);
	_staticMeshes.back()->_vertices.reserve(grassPositions.size() * 8);

	std::uniform_real_distribution<float> distRad(0.0f, _piTimes2);

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

	_aabb._lowerBound = _aabb._upperBound = _chunkCorner;

	std::uniform_int_distribution<int> distTile(0, numTiles - 1);
	std::uniform_real_distribution<float> dist11(-1.0f, 1.0f);
	std::uniform_real_distribution<float> dist01(0.0f, 1.0f);

	float tInXInv = 1.0f / tilesInX;
	float tInYInv = 1.0f / tilesInY;

	size_t grassCounter = 0;

	for (size_t i = 0; i < grassPositions.size(); i++, grassCounter++) {
		if (_staticMeshes.back()->_indices.size() >= std::numeric_limits<staticMeshIndexType>().max()) {
			// New mesh
			_staticMeshes.push_back(std::shared_ptr<StaticMesh>(new StaticMesh()));

			_staticMeshes.back()->create(true);

			grassCounter = 0;
		}

		float angle = distRad(getScene()->_randomGenerator);
		Matrix4x4f transform(Matrix4x4f::translateMatrix(grassPositions[i]) * Matrix4x4f::rotateMatrixY(angle));

		size_t vertexIndex = grassCounter * 8;

		int tileIndex = distTile(getScene()->_randomGenerator);

		float tx = static_cast<float>(tileIndex % tilesInX);
		float ty = static_cast<float>(tileIndex / tilesInX);

		Vec3f skewVec(dist01(getScene()->_randomGenerator) * maxTopSkew * Vec3f(dist11(getScene()->_randomGenerator), 0.0f, dist11(getScene()->_randomGenerator)).normalized());

		for (size_t j = 0; j < 8; j++) {
			_staticMeshes.back()->_vertices.push_back(pge::StaticMesh::Vertex());

			_staticMeshes.back()->_vertices.back()._position = transform * quadPositions[j];

			_aabb.expand(_staticMeshes.back()->_vertices.back()._position);

			_staticMeshes.back()->_vertices.back()._normal = Quaternion(angle, Vec3f(0.0f, 1.0f, 0.0f)) * quadNormals[j];

			_staticMeshes.back()->_vertices.back()._texCoord = Vec2f((tx + quadTexCoords[j].x) * tInXInv, (ty + quadTexCoords[j].y) * tInYInv);

			// Apply skew
			switch (j) {
			case 2:
			case 3:
			case 6:
			case 7:
				_staticMeshes.back()->_vertices.back()._position += skewVec;

				break;
			}
		}

		for (size_t j = 0; j < 12; j++)
			_staticMeshes.back()->_indices.push_back(vertexIndex + quadIndices[j]);
	}

	for (size_t i = 0; i < _staticMeshes.size(); i++)
		_staticMeshes[i]->updateBuffers();

	_aabb.calculateHalfDims();
	_aabb.calculateCenter();

	updateAABB();
}

void TerrainGrass::deferredRender() {
	assert(_grassBatchRenderer.isAlive());

	static_cast<TerrainGrassBatcher*>(_grassBatchRenderer.get())->_grassObjects.push_back(*this);
}