#pragma once

#include <pge/scene/RenderScene.h>
#include <pge/rendering/model/StaticPositionModel.h>

namespace pge {
	class SceneObjectStaticPositionModelBatcher : public SceneObject {
	private:
		std::unordered_map<StaticPositionModel*, std::list<Matrix4x4f>> _modelTransforms;

	public:
		SceneObjectStaticPositionModelBatcher() {
			_renderMask = 0xffff;
		}

		// Inherited from SceneObject
		void batchRender();

		SceneObjectStaticPositionModelBatcher* copyFactory() {
			return new SceneObjectStaticPositionModelBatcher(*this);
		}

		friend class StaticModel;
	};
}