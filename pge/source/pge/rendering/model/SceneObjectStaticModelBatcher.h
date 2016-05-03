#pragma once

#include <pge/scene/RenderScene.h>
#include <pge/rendering/model/StaticModel.h>

namespace pge {
	class SceneObjectStaticModelBatcher : public SceneObject {
	private:
		std::unordered_map<StaticModel*, std::list<Matrix4x4f>> _modelTransforms;

	public:
		SceneObjectStaticModelBatcher() {
			_renderMask = 0xffff;
		}

		// Inherited from SceneObject
		void batchRender();

		SceneObjectStaticModelBatcher* copyFactory() {
			return new SceneObjectStaticModelBatcher(*this);
		}

		friend class StaticModel;
	};
}