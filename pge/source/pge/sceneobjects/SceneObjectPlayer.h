#pragma once

#include "../scene/Scene.h"
#include "input/SceneObjectBufferedInput.h"
#include "physics/SceneObjectPhysicsWorld.h"
#include "../physics/DynamicCharacterController.h"

class SceneObjectPlayer : public pge::SceneObject {
private:
	pge::SceneObjectRef input;

	std::shared_ptr<pge::DynamicCharacterController> characterController;

	float radius, height, mass, stepHeight;

	sf::Vector2i lastMousePosition;

public:
	float sensitivity;

	float noClipAcceleration;
	float noClipDeceleration;

	float noClipRunMultiplier;

	float acceleration;
	float deceleration;

	float runMultiplier;

	float cameraHeightOffset;

	float angleX, angleY;

	pge::Vec3f noClipVelocity;

	bool allowNoclipChange;

	bool acceptingInput;

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