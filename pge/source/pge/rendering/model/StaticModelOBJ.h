#pragma once

#include <pge/assetmanager/Asset.h>
#include <pge/rendering/model/StaticModel.h>

namespace pge {
	class StaticModelOBJ : public Asset {
	private:
		AABB3D _aabb;

	public:
		AssetManager _textureManager;

		StaticModel _model;

		// Inherited from Asset
		bool createAsset(const std::string &name);

		void render(RenderScene* pScene) {
			_model.render(pScene);
		}

		void render(SceneObjectStaticModelBatcher* pBatcher, const Matrix4x4f &transform) {
			_model.render(pBatcher, transform);
		}

		const AABB3D &getAABB() const {
			return _aabb;
		}

		// Asset factory
		static Asset* assetFactory() {
			return new StaticModelOBJ();
		}
	};
}