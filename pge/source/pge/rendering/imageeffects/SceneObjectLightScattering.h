#pragma once

#include <pge/scene/RenderScene.h>
#include <pge/rendering/imageeffects/SceneObjectEffectBuffer.h>

namespace pge {
	class SceneObjectLightScattering : public SceneObject {
	private:
		pge::SceneObjectRef _effectBuffer;

		std::shared_ptr<Shader> _blurShaderHorizontal;
		std::shared_ptr<Shader> _blurShaderVertical;

		std::shared_ptr<Shader> _lightScatteringShader;

		std::shared_ptr<Shader> _renderImageShader;

	public:
		Vec3f _lightSourcePosition;
		Vec3f _lightSourceColor;

		size_t _numBlurPasses;
		float _blurRadius;

		SceneObjectLightScattering()
			: _lightSourcePosition(0.0f, 0.0f, 0.0f), _lightSourceColor(1.0f, 1.0f, 1.0f),
			_numBlurPasses(1), _blurRadius(0.00493f)
		{}

		void create(const std::shared_ptr<Shader> &blurShaderHorizontal,
			const std::shared_ptr<Shader> &blurShaderVertical,
			const std::shared_ptr<Shader> &lightScatteringShader,
			const std::shared_ptr<Shader> &renderImageShader);

		// Inherited from SceneObject
		void onAdd();
		void postRender();

		SceneObjectLightScattering* copyFactory() {
			return new SceneObjectLightScattering(*this);
		}
	};
}