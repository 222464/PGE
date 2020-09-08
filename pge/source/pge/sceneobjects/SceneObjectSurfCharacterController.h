#pragma once

#include "../scene/Scene.h"

#include "../constructs/Vec2f.h"

#include "physics/SceneObjectPhysicsWorld.h"

class IgnoreBodyAndGhostCast : public btCollisionWorld::ClosestRayResultCallback {
private:
	btRigidBody* pBody;
	btPairCachingGhostObject* pGhostObject;

public:
	IgnoreBodyAndGhostCast(btRigidBody* pBody, btPairCachingGhostObject* pGhostObject)
		: btCollisionWorld::ClosestRayResultCallback(btVector3(0.0, 0.0, 0.0), btVector3(0.0, 0.0, 0.0)),
		pBody(pBody), pGhostObject(pGhostObject)
	{}

	btScalar addSingleResult(btCollisionWorld::LocalRayResult &rayResult, bool normalInWorldSpace) {
		if (rayResult.m_collisionObject == pBody || rayResult.m_collisionObject == pGhostObject)
			return 1.0f;

		return ClosestRayResultCallback::addSingleResult(rayResult, normalInWorldSpace);
	}
};

class SceneObjectSurfCharacterController : public pge::SceneObject {
private:
	pge::SceneObjectRef input;

	pge::SceneObjectRef physicsWorld;

	std::shared_ptr<btCollisionShape> pCollisionShape;
	std::shared_ptr<btDefaultMotionState> pMotionState;
	std::shared_ptr<btRigidBody> pRigidBody;
	std::shared_ptr<btPairCachingGhostObject> pGhostObject;

	bool onGround;
	bool hittingWall;

	float bottomYOffset;
	float bottomRoundedRegionYOffset;

	btTransform motionTransform;

	pge::Vec3f manualVelocity;

	btVector3 previousPosition;

	float jumpRechargeTimer;

	bool mustCrouchFlag;

	pge::Vec3f floorNormal;

	void parseGhostContacts(std::vector<pge::Vec3f> &surfaceHitNormals);

	void updateVelocity(const std::vector<pge::Vec3f> &surfaceHitNormals, float dt);
	void updatePosition(float dt);

public:
	float deceleration;
	float maxSpeed;
	float jumpImpulse;

	float jumpRechargeTime;

	float stepHeight;

	// Camera
	float sensitivity;

	float angleX, angleY;

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
		return cons(pRigidBody->getWorldTransform().getOrigin());
	}

	void setPosition(const pge::Vec3f &position) {
		pRigidBody->getWorldTransform().setOrigin(bt(position));
	}

	pge::Vec3f getLinearVelocity() const {
		return cons(pRigidBody->getLinearVelocity());
	}

	void setLinearVelocity(const pge::Vec3f &velocity) {
		pRigidBody->setLinearVelocity(bt(velocity));
	}

	bool isOnGround() const {
		return onGround;
	}

	bool mustCrouch() const {
		return mustCrouchFlag;
	}

	btRigidBody* getRigidBody() const {
		return pRigidBody.get();
	}

	btPairCachingGhostObject* getGhostObject() const {
		return pGhostObject.get();
	}

	SceneObject* copyFactory() {
		return new SceneObjectSurfCharacterController(*this);
	}
};