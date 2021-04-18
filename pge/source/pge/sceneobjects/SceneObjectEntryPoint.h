#pragma once

#include "../scene/Scene.h"

class SceneObjectEntryPoint: public pge::SceneObject {
public:
    // Inherited from SceneObject
    void onAdd();

    SceneObject* copyFactory() {
        return new SceneObjectEntryPoint(*this);
    }
};
