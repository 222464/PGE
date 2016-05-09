#pragma once

#include <SFML/Network.hpp>

#include <pge/scene/RenderScene.h>

#include <pge/sceneobjects/SceneObjectProp.h>
#include <pge/sceneobjects/physics/SceneObjectPhysicsWorld.h>

#include <BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.h>

class SceneObjectQuadruped : public pge::SceneObject {
public:
	struct Limb {
		// Body
		std::shared_ptr<btCollisionShape> _pCollisionShape;
		std::shared_ptr<btDefaultMotionState> _pMotionState;
		std::shared_ptr<btRigidBody> _pRigidBody;

		// Joint
		std::shared_ptr<btGeneric6DofConstraint> _pConstraint;
	};

	struct Leg {
		Limb _lower;
		Limb _upper;

		std::shared_ptr<btGhostObject> _pGhostLower;

		void create(pge::SceneObjectPhysicsWorld* pPhysicsWorld, btRigidBody* pBodyPart, const btVector3 &rootPos);
	};

private:
	// Rendering
	pge::StaticModelOBJ* _pBodyPartModel;
	pge::StaticModelOBJ* _pLimbModel;

	pge::SceneObjectRef _batcherRef;

	std::array<float, 27> _action;

	int _ticksPerAction;
	int _ticks;

	pge::SceneObjectRef _orbCam;

	// Physics
	pge::SceneObjectRef _physicsWorld;

	// Floor
	std::shared_ptr<btCollisionShape> _pCollisionShapeFloor;
	std::shared_ptr<btDefaultMotionState> _pMotionStateFloor;
	std::shared_ptr<btRigidBody> _pRigidBodyFloor;

	// Body forward
	std::shared_ptr<btCollisionShape> _pCollisionShapeBodyForward;
	std::shared_ptr<btDefaultMotionState> _pMotionStateBodyForward;
	std::shared_ptr<btRigidBody> _pRigidBodyForward;

	// Body backward
	std::shared_ptr<btCollisionShape> _pCollisionShapeBodyBackward;
	std::shared_ptr<btDefaultMotionState> _pMotionStateBodyBackward;
	std::shared_ptr<btRigidBody> _pRigidBodyBackward;

	// Joint between forward and backward body parts
	std::shared_ptr<btGeneric6DofConstraint> _pConstraintForwardBackward;

	// Limbs
	std::array<Leg, 4> _legs;

	float _reward;

	float _prevDist;

	bool _show;

	bool _doneLastFrame;

	std::vector<int> _blocks;

	std::mt19937 _rng;

	bool _capture;
	std::shared_ptr<std::vector<char>> _capBytes;

	std::shared_ptr<sf::TcpSocket> _socket;

	void act(float dt);

public:
	static const unsigned short _port = 54003;

	static const unsigned int _maxBatchSize = 16384;

	static const unsigned int _gameSeed = 1234;

	SceneObjectQuadruped() {
		_renderMask = 0xffff;
	}

	bool create();

	// Inherited from SceneObject
	void onAdd();
	void synchronousUpdate(float dt);
	void deferredRender();
	void onDestroy();
	void postRender();

	pge::Vec3f getPosition() const {
		return cons(_pRigidBodyForward->getWorldTransform().getOrigin());
	}

	void reset();

	SceneObjectQuadruped* copyFactory() {
		return new SceneObjectQuadruped(*this);
	}
};

