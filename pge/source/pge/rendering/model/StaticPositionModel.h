#pragma once

#include <pge/rendering/mesh/StaticPositionMesh.h>
#include <pge/rendering/material/Material.h>

#include <pge/scene/RenderScene.h>

namespace pge {
	class StaticPositionModel {
	public:
		std::vector<std::shared_ptr<StaticPositionMesh>> _meshes;

		void render();

		// Loaders
		bool loadFromOBJ(const std::string &fileName, AABB3D &aabb, bool useBuffers, bool clearArrays);
	};
}