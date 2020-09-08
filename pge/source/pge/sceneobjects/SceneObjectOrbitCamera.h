#pragma once

#include "../scene/Scene.h"

#include "input/SceneObjectBufferedInput.h"

class SceneObjectOrbitCamera : public pge::SceneObject {
private:
	pge::SceneObjectRef input;

public:
	pge::Vec3f focusPoint;

	float distance;
	float tilt;

	float angle;

	float angularVelocity;

	SceneObjectOrbitCamera();

	// Inherited from SceneObject
	void onAdd();
	void update(float dt);

	SceneObject* copyFactory() {
		return new SceneObjectOrbitCamera(*this);
	}
};