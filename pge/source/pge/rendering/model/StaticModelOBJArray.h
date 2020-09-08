#pragma once

#include "../../assetmanager/Asset.h"
#include "StaticModel.h"

namespace pge {
	class StaticModelOBJArray : public Asset {
	private:
		AABB3D aabb;

	public:
		AssetManager textureManager;

		StaticModel model;

		// Inherited from Asset
		bool createAsset(const std::string &name);

		void render(RenderScene* pScene) {
			model.render(pScene);
		}

		void render(SceneObjectStaticModelBatcher* pBatcher, const Matrix4x4f &transform) {
			model.render(pBatcher, transform);
		}

		const AABB3D &getAABB() const {
			return aabb;
		}

		// Asset factory
		static Asset* assetFactory() {
			return new StaticModelOBJArray();
		}
	};
}