#pragma once

#include <pge/scene/Scene.h>
#include <pge/sceneobjects/input/SceneObjectBufferedInput.h>
#include <pge/sceneobjects/physics/SceneObjectPhysicsWorld.h>
#include <pge/physics/DynamicCharacterController.h>

class SceneObjectPlayer : public pge::SceneObject {
private:
	pge::SceneObjectRef _input;

	std::shared_ptr<pge::DynamicCharacterController> _characterController;

	float _radius, _height, _mass, _stepHeight;

	sf::Vector2i _lastMousePosition;

public:
	float _sensitivity;

	float _noClipAcceleration;
	float _noClipDeceleration;

	float _noClipRunMultiplier;

	float _acceleration;
	float _deceleration;

	float _runMultiplier;

	float _cameraHeightOffset;

	float _angleX, _angleY;

	pge::Vec3f _noClipVelocity;

	bool _allowNoclipChange;

	bool _acceptingInput;

	SceneObjectPlayer();

	void setNoClip(bool noClip);

	// Inherited from SceneObject
	void onAdd();
	void update(float dt);
	void synchronousUpdate(float dt);

	SceneObjectPlayer* copyFactory() {
		return new SceneObjectPlayer(*this);
	}
};