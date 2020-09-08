#pragma once

#include "../../scene/RenderScene.h"
#include "SceneObjectEffectBuffer.h"

namespace pge {
	class SceneObjectFXAA : public SceneObject {
	private:
		std::shared_ptr<Shader> fxaaShader;
		std::shared_ptr<Shader> lumaShader;

		std::shared_ptr<TextureRT> lumaBuffer;

	public:
		void create(const std::shared_ptr<Shader> &fxaaShader, const std::shared_ptr<Shader> &lumaShader);

		// Inherited from SceneObject
		void postRender();

		SceneObjectFXAA* copyFactory() {
			return new SceneObjectFXAA(*this);
		}
	};
}