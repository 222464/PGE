#pragma once

#include "../../scene/RenderScene.h"
#include "SceneObjectEffectBuffer.h"

namespace pge {
	class SceneObjectLightScattering : public SceneObject {
	private:
		pge::SceneObjectRef effectBuffer;

		std::shared_ptr<Shader> blurShaderHorizontal;
		std::shared_ptr<Shader> blurShaderVertical;

		std::shared_ptr<Shader> lightScatteringShader;

		std::shared_ptr<Shader> renderImageShader;

	public:
		Vec3f lightSourcePosition;
		Vec3f lightSourceColor;

		size_t numBlurPasses;
		float blurRadius;

		SceneObjectLightScattering()
			: lightSourcePosition(0.0f, 0.0f, 0.0f), lightSourceColor(1.0f, 1.0f, 1.0f),
			numBlurPasses(1), blurRadius(0.00493f)
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