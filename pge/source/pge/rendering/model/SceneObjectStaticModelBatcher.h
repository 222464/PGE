#pragma once

#include "../../scene/RenderScene.h"
#include "StaticModel.h"

namespace pge {
    class SceneObjectStaticModelBatcher : public SceneObject {
    private:
        std::unordered_map<StaticModel*, std::list<Matrix4x4f>> modelTransforms;

    public:
        SceneObjectStaticModelBatcher() {
            renderMask = 0xffff;
        }

        // Inherited from SceneObject
        void batchRender();

        SceneObjectStaticModelBatcher* copyFactory() {
            return new SceneObjectStaticModelBatcher(*this);
        }

        friend class StaticModel;
    };
}