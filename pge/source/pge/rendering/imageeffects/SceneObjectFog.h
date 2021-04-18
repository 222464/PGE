#pragma once

#include "../../scene/RenderScene.h"
#include "SceneObjectEffectBuffer.h"

namespace pge {
    class SceneObjectFog : public SceneObject {
    private:
        std::shared_ptr<Shader> fogShader;

    public:
        Vec3f fogColor;
        float fogStartDistance;

        SceneObjectFog()
            : fogColor(0.02f, 0.02f, 0.02f), fogStartDistance(0.0f)
        {
            renderMask = 0xffff;
        }

        void create(const std::shared_ptr<Shader> &fogShader);

        // Inherited from SceneObject
        void postRender();

        SceneObjectFog* copyFactory() {
            return new SceneObjectFog(*this);
        }
    };
}