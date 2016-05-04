#pragma once

#include <pge/scene/Scene.h>

#include <pge/sceneobjects/input/SceneObjectBufferedInput.h>

class SceneObjectOrbitCamera : public pge::SceneObject {
private:
	pge::SceneObjectRef _input;

public:
	pge::Vec3f _focusPoint;

	float _distance;
	float _tilt;

	float _angle;

	float _angularVelocity;

	SceneObjectOrbitCamera();

	// Inherited from SceneObject
	void onAdd();
	void update(float dt);

	SceneObject* copyFactory() {
		return new SceneObjectOrbitCamera(*this);
	}
};