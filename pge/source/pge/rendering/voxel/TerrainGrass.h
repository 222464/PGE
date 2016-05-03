#pragma once

#include <pge/scene/SceneObject.h>
#include <pge/rendering/mesh/StaticMesh.h>

namespace pge {
	class TerrainGrass : public SceneObject {
	private:
		SceneObjectRef _grassBatchRenderer;

		std::vector<std::shared_ptr<StaticMesh>> _staticMeshes;

		Vec3f _chunkCorner;

	public:
		void create(const std::vector<Vec3f> &grassPositions, const Vec3f &chunkCorner,
			const SceneObjectRef &grassBatchRenderer, int tilesInX, int tilesInY, int numTiles, float maxTopSkew);

		// Inherited from SceneObject
		void deferredRender();

		SceneObject* copyFactory() {
			return new TerrainGrass(*this);
		}

		friend class TerrainGrassBatcher;
	};
}