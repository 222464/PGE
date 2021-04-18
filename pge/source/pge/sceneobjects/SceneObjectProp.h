#pragma once

#include "../scene/Scene.h"

#include "../constructs/Matrix4x4f.h"

#include "../rendering/model/StaticModelOBJ.h"

#include <iostream>

class SceneObjectProp : public pge::SceneObject {
private:
    pge::StaticModelOBJ* pModelOBJ;

    pge::SceneObjectRef batcherRef;

public:
    pge::Matrix4x4f transform;

    SceneObjectProp() {
        renderMask = 0xffff;
    }

    bool create(const std::string &fileName);

    void calculateAABB();

    // Inherited from SceneObject
    void onAdd();
    void deferredRender();

    pge::StaticModelOBJ* getModel() {
        return pModelOBJ;
    }

    SceneObject* copyFactory() {
        return new SceneObjectProp(*this);
    }
};