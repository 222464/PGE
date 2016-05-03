#pragma once

#include <pge/scene/RenderScene.h>
#include <pge/rendering/imageeffects/SceneObjectEffectBuffer.h>

namespace pge {
	class SceneObjectFog : public SceneObject {
	private:
		std::shared_ptr<Shader> _fogShader;

	public:
		Vec3f _fogColor;
		float _fogStartDistance;

		SceneObjectFog()
			: _fogColor(0.02f, 0.02f, 0.02f), _fogStartDistance(0.0f)
		{
			_renderMask = 0xffff;
		}

		void create(const std::shared_ptr<Shader> &fogShader);

		// Inherited from SceneObject
		void postRender();

		SceneObjectFog* copyFactory() {
			return new SceneObjectFog(*this);
		}
	};
}