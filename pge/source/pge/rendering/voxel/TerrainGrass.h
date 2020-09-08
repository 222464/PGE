#pragma once

#include "../../scene/SceneObject.h"
#include "../mesh/StaticMesh.h"

namespace pge {
	class TerrainGrass : public SceneObject {
	private:
		SceneObjectRef grassBatchRenderer;

		std::vector<std::shared_ptr<StaticMesh>> staticMeshes;

		Vec3f chunkCorner;

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