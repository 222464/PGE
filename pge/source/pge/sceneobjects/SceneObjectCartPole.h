#pragma once

#include <SFML/Network.hpp>

#include "../scene/RenderScene.h"

#include "SceneObjectProp.h"
#include "physics/SceneObjectPhysicsWorld.h"

#include <BulletDynamics/ConstraintSolver/btConeTwistConstraint.h>

class SceneObjectCartPole : public pge::SceneObject {
private:
	// Rendering
	pge::StaticModelOBJ* pCartModel;
	pge::StaticModelOBJ* pPoleModel;

	pge::SceneObjectRef batcherRef;

	pge::Vec2f action;

	int ticksPerAction;
	int ticks;

	// Physics
	pge::SceneObjectRef physicsWorld;

	// Floor
	std::shared_ptr<btCollisionShape> pCollisionShapeFloor;
	std::shared_ptr<btDefaultMotionState> pMotionStateFloor;
	std::shared_ptr<btRigidBody> pRigidBodyFloor;

	// Cart
	std::shared_ptr<btCollisionShape> pCollisionShapeCart;
	std::shared_ptr<btDefaultMotionState> pMotionStateCart;
	std::shared_ptr<btRigidBody> pRigidBodyCart;

	// Pole
	std::shared_ptr<btCollisionShape> pCollisionShapePole;
	std::shared_ptr<btDefaultMotionState> pMotionStatePole;
	std::shared_ptr<btRigidBody> pRigidBodyPole;

	// Joint
	std::shared_ptr<btConeTwistConstraint> pConstraint;

	float reward;

	bool show;

	bool doneLastFrame;

	std::mt19937 rng;

	bool capture;
	std::shared_ptr<std::vector<char>> capBytes;

	std::shared_ptr<sf::TcpSocket> socket;

	void act();

public:
	static const unsigned short port = 54003;

	static const unsigned int maxBatchSize = 16384;

	static const unsigned int gameSeed = 1234;

	SceneObjectCartPole() {
		renderMask = 0xffff;
	}

	bool create();

	// Inherited from SceneObject
	void onAdd();
	void synchronousUpdate(float dt);
	void deferredRender();
	void onDestroy();
	void postRender();

	void reset();

	SceneObjectCartPole* copyFactory() {
		return new SceneObjectCartPole(*this);
	}
};

