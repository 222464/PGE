#pragma once

#include "../../scene/RenderScene.h"
#include "SceneObjectEffectBuffer.h"

namespace pge {
	class SceneObjectSSAO : public SceneObject {
	private:
		pge::SceneObjectRef effectBuffer;

		std::shared_ptr<Shader> blurShaderHorizontal;
		std::shared_ptr<Shader> blurShaderVertical;

		std::shared_ptr<Shader> ssaoShader;

		std::shared_ptr<Shader> renderImageShader;

		std::shared_ptr<Texture2D> noiseMap;

	public:
		size_t numBlurPasses;
		float blurRadius;

		float ssaoRadius;
		float ssaoStrength;

		SceneObjectSSAO()
			: numBlurPasses(2), blurRadius(0.00493f), ssaoRadius(0.2f), ssaoStrength(3.0f)
		{}

		void create(const std::shared_ptr<Shader> &blurShaderHorizontal,
			const std::shared_ptr<Shader> &blurShaderVertical,
			const std::shared_ptr<Shader> &ssaoShader,
			const std::shared_ptr<Shader> &renderImageShader,
			const std::shared_ptr<Texture2D> &noiseMap);

		// Inherited from SceneObject
		void onAdd();
		void postRender();

		SceneObjectSSAO* copyFactory() {
			return new SceneObjectSSAO(*this);
		}
	};
}