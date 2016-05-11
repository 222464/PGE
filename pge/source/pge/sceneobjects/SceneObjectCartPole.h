#pragma once

#include <SFML/Network.hpp>

#include <pge/scene/RenderScene.h>

#include <pge/sceneobjects/SceneObjectProp.h>
#include <pge/sceneobjects/physics/SceneObjectPhysicsWorld.h>

#include <BulletDynamics/ConstraintSolver/btConeTwistConstraint.h>

class SceneObjectCartPole : public pge::SceneObject {
private:
	// Rendering
	pge::StaticModelOBJ* _pCartModel;
	pge::StaticModelOBJ* _pPoleModel;

	pge::SceneObjectRef _batcherRef;

	pge::Vec2f _action;

	int _ticksPerAction;
	int _ticks;

	// Physics
	pge::SceneObjectRef _physicsWorld;

	// Floor
	std::shared_ptr<btCollisionShape> _pCollisionShapeFloor;
	std::shared_ptr<btDefaultMotionState> _pMotionStateFloor;
	std::shared_ptr<btRigidBody> _pRigidBodyFloor;

	// Cart
	std::shared_ptr<btCollisionShape> _pCollisionShapeCart;
	std::shared_ptr<btDefaultMotionState> _pMotionStateCart;
	std::shared_ptr<btRigidBody> _pRigidBodyCart;

	// Pole
	std::shared_ptr<btCollisionShape> _pCollisionShapePole;
	std::shared_ptr<btDefaultMotionState> _pMotionStatePole;
	std::shared_ptr<btRigidBody> _pRigidBodyPole;

	// Joint
	std::shared_ptr<btConeTwistConstraint> _pConstraint;

	float _reward;

	bool _show;

	bool _doneLastFrame;

	std::mt19937 _rng;

	bool _capture;
	std::shared_ptr<std::vector<char>> _capBytes;

	std::shared_ptr<sf::TcpSocket> _socket;

	void act();

public:
	static const unsigned short _port = 54003;

	static const unsigned int _maxBatchSize = 16384;

	static const unsigned int _gameSeed = 1234;

	SceneObjectCartPole() {
		_renderMask = 0xffff;
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

