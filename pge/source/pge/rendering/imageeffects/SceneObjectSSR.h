#pragma once

#include "../../scene/RenderScene.h"
#include "SceneObjectEffectBuffer.h"
#include "../texture/TextureCube.h"

namespace pge {
    class SceneObjectSSR : public SceneObject {
    private:
        pge::SceneObjectRef effectBuffer;

        std::shared_ptr<Shader> blurShaderHorizontalEdgeAware;
        std::shared_ptr<Shader> blurShaderVerticalEdgeAware;

        std::shared_ptr<Shader> ssrShader;

        std::shared_ptr<Shader> renderImageShader;

        std::shared_ptr<TextureCube> cubeMap;

        std::shared_ptr<Texture2D> noiseMap;

    public:
        size_t numBlurPasses;
        float blurRadius;

        float reflectionRadius;

        SceneObjectSSR()
            : numBlurPasses(3), blurRadius(0.001727f)
        {}

        void create(const std::shared_ptr<Shader> &blurShaderHorizontalEdgeAware,
            const std::shared_ptr<Shader> &blurShaderVerticalEdgeAware,
            const std::shared_ptr<Shader> &ssrShader,
            const std::shared_ptr<Shader> &renderImageShader,
            const std::shared_ptr<TextureCube> &cubeMap,
            const std::shared_ptr<Texture2D> &noiseMap);

        // Inherited from SceneObject
        void onAdd();
        void postRender();

        SceneObjectSSR* copyFactory() {
            return new SceneObjectSSR(*this);
        }
    };
}