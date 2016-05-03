#pragma once

#include <pge/scene/RenderScene.h>

namespace pge {
	class SceneObjectEffectBuffer : public SceneObject {
	private:
	public:
		std::shared_ptr<pge::TextureRT> _ping;
		std::shared_ptr<pge::TextureRT> _pong;

		std::shared_ptr<pge::TextureRT> _fullPing;
		std::shared_ptr<pge::TextureRT> _fullPong;

		void create(float downsampleRatio);

		SceneObjectEffectBuffer* copyFactory() {
			return new SceneObjectEffectBuffer(*this);
		}
	};
}