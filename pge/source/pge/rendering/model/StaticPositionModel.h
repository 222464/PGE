#pragma once

#include "../mesh/StaticPositionMesh.h"
#include "../material/Material.h"

#include "../../scene/RenderScene.h"

namespace pge {
	class StaticPositionModel {
	public:
		std::vector<std::shared_ptr<StaticPositionMesh>> meshes;

		void render();

		// Loaders
		bool loadFromOBJ(const std::string &fileName, AABB3D &aabb, bool useBuffers, bool clearArrays);
	};
}