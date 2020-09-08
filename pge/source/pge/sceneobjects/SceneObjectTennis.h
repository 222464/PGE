#pragma once

#include <SFML/Network.hpp>

#include "../scene/RenderScene.h"

#include "SceneObjectProp.h"
#include "physics/SceneObjectPhysicsWorld.h"

#include <BulletDynamics/ConstraintSolver/btConeTwistConstraint.h>

class SceneObjectTennis : public pge::SceneObject {
private:
	// Rendering
	pge::StaticModelOBJ* pSlimeModel;
	pge::StaticModelOBJ* pBallModel;

	pge::SceneObjectRef batcherRef;

	pge::Vec2f action;

	int ticksPerAction;
	int ticks;

	// Physics
	pge::SceneObjectRef physicsWorld;

	// Slime
	std::shared_ptr<btCollisionShape> pCollisionShapeSlime;
	std::shared_ptr<btDefaultMotionState> pMotionStateSlime;
	std::shared_ptr<btRigidBody> pRigidBodySlime;

	// Ball
	std::shared_ptr<btCollisionShape> pCollisionShapeBall;
	std::shared_ptr<btDefaultMotionState> pMotionStateBall;
	std::shared_ptr<btRigidBody> pRigidBodyBall;

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

	SceneObjectTennis() {
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

	SceneObjectTennis* copyFactory() {
		return new SceneObjectTennis(*this);
	}
};

