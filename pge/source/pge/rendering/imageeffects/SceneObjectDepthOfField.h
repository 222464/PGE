#pragma once

#include "../../scene/RenderScene.h"
#include "SceneObjectEffectBuffer.h"

namespace pge {
	class SceneObjectDepthOfField : public SceneObject {
	private:
		pge::SceneObjectRef effectBuffer;

		std::shared_ptr<Shader> depthOfFieldBlurShaderHorizontal;
		std::shared_ptr<Shader> depthOfFieldBlurShaderVertical;

		std::shared_ptr<Shader> renderImageShader;

	public:
		size_t numBlurPasses;
		float blurRadius;

		float focalDistance;
		float focalRange;

		SceneObjectDepthOfField()
			: focalDistance(8.0f), focalRange(2.0f),
			numBlurPasses(3), blurRadius(0.00115f)
		{}

		void create(const std::shared_ptr<Shader> &depthOfFieldBlurShaderHorizontal,
			const std::shared_ptr<Shader> &depthOfFieldBlurShaderVertical,
			const std::shared_ptr<Shader> &renderImageShader);

		// Inherited from SceneObject
		void onAdd();
		void postRender();

		SceneObjectDepthOfField* copyFactory() {
			return new SceneObjectDepthOfField(*this);
		}
	};
}