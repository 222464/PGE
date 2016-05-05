#pragma once

#include <pge/scene/Scene.h>

#include <pge/constructs/Vec2f.h>

#include <pge/sceneobjects/physics/SceneObjectPhysicsWorld.h>

class IgnoreBodyAndGhostCast : public btCollisionWorld::ClosestRayResultCallback {
private:
	btRigidBody* _pBody;
	btPairCachingGhostObject* _pGhostObject;

public:
	IgnoreBodyAndGhostCast(btRigidBody* pBody, btPairCachingGhostObject* pGhostObject)
		: btCollisionWorld::ClosestRayResultCallback(btVector3(0.0, 0.0, 0.0), btVector3(0.0, 0.0, 0.0)),
		_pBody(pBody), _pGhostObject(pGhostObject)
	{}

	btScalar addSingleResult(btCollisionWorld::LocalRayResult &rayResult, bool normalInWorldSpace) {
		if (rayResult.m_collisionObject == _pBody || rayResult.m_collisionObject == _pGhostObject)
			return 1.0f;

		return ClosestRayResultCallback::addSingleResult(rayResult, normalInWorldSpace);
	}
};

class SceneObjectSurfCharacterController : public pge::SceneObject {
private:
	pge::SceneObjectRef _input;

	pge::SceneObjectRef _physicsWorld;

	std::shared_ptr<btCollisionShape> _pCollisionShape;
	std::shared_ptr<btDefaultMotionState> _pMotionState;
	std::shared_ptr<btRigidBody> _pRigidBody;
	std::shared_ptr<btPairCachingGhostObject> _pGhostObject;

	bool _onGround;
	bool _hittingWall;

	float _bottomYOffset;
	float _bottomRoundedRegionYOffset;

	btTransform _motionTransform;

	pge::Vec3f _manualVelocity;

	btVector3 _previousPosition;

	float _jumpRechargeTimer;

	bool _mustCrouch;

	pge::Vec3f _floorNormal;

	void parseGhostContacts(std::vector<pge::Vec3f> &surfaceHitNormals);

	void updateVelocity(const std::vector<pge::Vec3f> &surfaceHitNormals, float dt);
	void updatePosition(float dt);

public:
	float _deceleration;
	float _maxSpeed;
	float _jumpImpulse;

	float _jumpRechargeTime;

	float _stepHeight;

	// Camera
	float _sensitivity;

	float _angleX, _angleY;

	SceneObjectSurfCharacterController();
	~SceneObjectSurfCharacterController();

	void create(pge::SceneObjectPhysicsWorld* pPhysicsWorld, const pge::Vec3f &spawnPos, float radius, float height, float mass, float stepHeight);

	// Acceleration vector in XZ plane
	void walk(const pge::Vec2f &dir);

	// Flattens vector
	void walk(const pge::Vec3f &dir) {
		walk(pge::Vec2f(dir.x, dir.z).normalized() * dir.magnitude());
	}

	void physUpdate(float dt);

	void jump();

	// Inherited from SceneObject
	void onAdd();
	void update(float dt);

	pge::Vec3f getPosition() const {
		return cons(_pRigidBody->getWorldTransform().getOrigin());
	}

	void setPosition(const pge::Vec3f &position) {
		_pRigidBody->getWorldTransform().setOrigin(bt(position));
	}

	pge::Vec3f getLinearVelocity() const {
		return cons(_pRigidBody->getLinearVelocity());
	}

	void setLinearVelocity(const pge::Vec3f &velocity) {
		_pRigidBody->setLinearVelocity(bt(velocity));
	}

	bool isOnGround() const {
		return _onGround;
	}

	bool mustCrouch() const {
		return _mustCrouch;
	}

	btRigidBody* getRigidBody() const {
		return _pRigidBody.get();
	}

	btPairCachingGhostObject* getGhostObject() const {
		return _pGhostObject.get();
	}

	SceneObject* copyFactory() {
		return new SceneObjectSurfCharacterController(*this);
	}
};