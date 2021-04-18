#pragma once

#include "../scene/Scene.h"

#include "input/SceneObjectBufferedInput.h"

class SceneObjectFloatingCamera : public pge::SceneObject {
private:
    pge::SceneObjectRef input;

public:
    float sensitivity;
    float acceleration;
    float deceleration;

    float runMultiplier;

    float angleX, angleY;

    pge::Vec3f velocity;

    bool acceptingInput;

    SceneObjectFloatingCamera();

    // Inherited from SceneObject
    void onAdd();
    void update(float dt);

    SceneObject* copyFactory() {
        return new SceneObjectFloatingCamera(*this);
    }
};