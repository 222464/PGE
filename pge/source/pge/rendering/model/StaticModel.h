#pragma once

#include "../mesh/StaticMesh.h"
#include "../material/Material.h"

#include "../../scene/RenderScene.h"

// Uncomment line below to flip texture coordinates across Y
#define PGE_OBJ_FLIP_TEXCOORD_Y

namespace pge {
	struct StaticMeshIndexSet {
		staticMeshIndexType vi, ti, ni;

		// Custom hash stuff - if this index set already exists, must have a vertex for it
		size_t operator()(const StaticMeshIndexSet &set) const {
			return static_cast<size_t>(set.vi ^ set.ti ^ set.ni);
		}

		bool operator==(const StaticMeshIndexSet &other) const {
			return vi == other.vi && ti == other.ti && ni == other.ni;
		}
	};

	class StaticModel {
	public:
		struct StaticMeshAndMaterialIndex {
			std::shared_ptr<StaticMesh> mesh;
			size_t materialIndex;

			StaticMeshAndMaterialIndex() {}
			StaticMeshAndMaterialIndex(const std::shared_ptr<StaticMesh> &mesh, size_t materialIndex)
				: mesh(mesh), materialIndex(materialIndex)
			{}
		};

		std::vector<StaticMeshAndMaterialIndex> meshes;
		std::vector<Material> materials;

		void render(RenderScene* pScene);
		void render(class SceneObjectStaticModelBatcher* pBatcher, const Matrix4x4f &transform);

		void genMipMaps();

		// Loaders
		bool loadFromOBJ(const std::string &fileName, AssetManager &textureManager, AABB3D &aabb, bool useBuffers, bool clearArrays, RenderScene* pRenderScene = nullptr);
	};
}