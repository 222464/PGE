#pragma once

#include "../../scene/RenderScene.h"
#include "StaticPositionModel.h"

namespace pge {
    class SceneObjectStaticPositionModelBatcher : public SceneObject {
    private:
        std::unordered_map<StaticPositionModel*, std::list<Matrix4x4f>> modelTransforms;

    public:
        SceneObjectStaticPositionModelBatcher() {
            renderMask = 0xffff;
        }

        // Inherited from SceneObject
        void batchRender();

        SceneObjectStaticPositionModelBatcher* copyFactory() {
            return new SceneObjectStaticPositionModelBatcher(*this);
        }

        friend class StaticModel;
    };
}