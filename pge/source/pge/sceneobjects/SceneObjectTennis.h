#pragma once

#include <SFML/Network.hpp>

#include <pge/scene/RenderScene.h>

#include <pge/sceneobjects/SceneObjectProp.h>
#include <pge/sceneobjects/physics/SceneObjectPhysicsWorld.h>

#include <BulletDynamics/ConstraintSolver/btConeTwistConstraint.h>

class SceneObjectTennis : public pge::SceneObject {
private:
	// Rendering
	pge::StaticModelOBJ* _pSlimeModel;
	pge::StaticModelOBJ* _pBallModel;

	pge::SceneObjectRef _batcherRef;

	pge::Vec2f _action;

	int _ticksPerAction;
	int _ticks;

	// Physics
	pge::SceneObjectRef _physicsWorld;

	// Slime
	std::shared_ptr<btCollisionShape> _pCollisionShapeSlime;
	std::shared_ptr<btDefaultMotionState> _pMotionStateSlime;
	std::shared_ptr<btRigidBody> _pRigidBodySlime;

	// Ball
	std::shared_ptr<btCollisionShape> _pCollisionShapeBall;
	std::shared_ptr<btDefaultMotionState> _pMotionStateBall;
	std::shared_ptr<btRigidBody> _pRigidBodyBall;

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

	SceneObjectTennis() {
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

	SceneObjectTennis* copyFactory() {
		return new SceneObjectTennis(*this);
	}
};

