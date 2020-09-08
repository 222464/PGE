#pragma once

#include "../../scene/RenderScene.h"

namespace pge {
	class SceneObjectEffectBuffer : public SceneObject {
	private:
	public:
		std::shared_ptr<pge::TextureRT> ping;
		std::shared_ptr<pge::TextureRT> pong;

		std::shared_ptr<pge::TextureRT> fullPing;
		std::shared_ptr<pge::TextureRT> fullPong;

		void create(float downsampleRatio);

		SceneObjectEffectBuffer* copyFactory() {
			return new SceneObjectEffectBuffer(*this);
		}
	};
}