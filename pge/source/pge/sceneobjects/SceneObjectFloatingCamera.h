#pragma once

#include <pge/scene/Scene.h>

#include <pge/sceneobjects/input/SceneObjectBufferedInput.h>

class SceneObjectFloatingCamera : public pge::SceneObject {
private:
	pge::SceneObjectRef _input;

public:
	float _sensitivity;
	float _acceleration;
	float _deceleration;

	float _runMultiplier;

	float _angleX, _angleY;

	pge::Vec3f _velocity;

	bool _acceptingInput;

	SceneObjectFloatingCamera();

	// Inherited from SceneObject
	void onAdd();
	void update(float dt);

	SceneObject* copyFactory() {
		return new SceneObjectFloatingCamera(*this);
	}
};