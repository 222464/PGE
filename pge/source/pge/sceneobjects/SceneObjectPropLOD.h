#pragma once

#include "../scene/Scene.h"

#include "../constructs/Matrix4x4f.h"

#include "../rendering/model/StaticModelOBJ.h"

#include <iostream>

class SceneObjectPropLOD : public pge::SceneObject {
private:
    std::vector<pge::StaticModelOBJ*> pModelsOBJ;

    pge::SceneObjectRef batcherRef;

public:
    pge::Matrix4x4f transform;

    float lodSwitchDistance;

    SceneObjectPropLOD()
        : lodSwitchDistance(20.0f)
    {
        renderMask = 0xffff;
    }

    bool create(const std::vector<std::string> &fileNames);

    void calculateAABB();

    // Inherited from SceneObject
    void onAdd();
    void deferredRender();

    SceneObject* copyFactory() {
        return new SceneObjectPropLOD(*this);
    }
};