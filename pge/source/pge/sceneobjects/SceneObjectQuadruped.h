#pragma once

#include <SFML/Network.hpp>

#include "../scene/RenderScene.h"

#include "SceneObjectProp.h"
#include "physics/SceneObjectPhysicsWorld.h"

#include <BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.h>

class SceneObjectQuadruped : public pge::SceneObject {
public:
	struct Limb {
		// Body
		std::shared_ptr<btCollisionShape> pCollisionShape;
		std::shared_ptr<btDefaultMotionState> pMotionState;
		std::shared_ptr<btRigidBody> pRigidBody;

		// Joint
		std::shared_ptr<btGeneric6DofConstraint> pConstraint;
	};

	struct Leg {
		Limb lower;
		Limb upper;

		std::shared_ptr<btGhostObject> pGhostLower;

		void create(pge::SceneObjectPhysicsWorld* pPhysicsWorld, btRigidBody* pBodyPart, const btVector3 &rootPos);
	};

private:
	// Rendering
	pge::StaticModelOBJ* pBodyPartModel;
	pge::StaticModelOBJ* pLimbModel;

	pge::SceneObjectRef batcherRef;

	std::array<float, 27> action;

	int ticksPerAction;
	int ticks;

	pge::SceneObjectRef orbCam;

	// Physics
	pge::SceneObjectRef physicsWorld;

	// Floor
	std::shared_ptr<btCollisionShape> pCollisionShapeFloor;
	std::shared_ptr<btDefaultMotionState> pMotionStateFloor;
	std::shared_ptr<btRigidBody> pRigidBodyFloor;

	// Body forward
	std::shared_ptr<btCollisionShape> pCollisionShapeBodyForward;
	std::shared_ptr<btDefaultMotionState> pMotionStateBodyForward;
	std::shared_ptr<btRigidBody> pRigidBodyForward;

	// Body backward
	std::shared_ptr<btCollisionShape> pCollisionShapeBodyBackward;
	std::shared_ptr<btDefaultMotionState> pMotionStateBodyBackward;
	std::shared_ptr<btRigidBody> pRigidBodyBackward;

	// Joint between forward and backward body parts
	std::shared_ptr<btGeneric6DofConstraint> pConstraintForwardBackward;

	// Limbs
	std::array<Leg, 4> legs;

	float reward;

	float prevDist;

	bool show;

	bool doneLastFrame;

	std::mt19937 rng;

	bool capture;
	std::shared_ptr<std::vector<char>> capBytes;

	std::shared_ptr<sf::TcpSocket> socket;

	void act(float dt);

public:
	static const unsigned short port = 54003;

	static const unsigned int maxBatchSize = 16384;

	static const unsigned int gameSeed = 1234;

	SceneObjectQuadruped() {
		renderMask = 0xffff;
	}

	bool create();

	// Inherited from SceneObject
	void onAdd();
	void synchronousUpdate(float dt);
	void deferredRender();
	void onDestroy();
	void postRender();

	pge::Vec3f getPosition() const {
		return cons(pRigidBodyForward->getWorldTransform().getOrigin());
	}

	void reset();

	SceneObjectQuadruped* copyFactory() {
		return new SceneObjectQuadruped(*this);
	}
};

