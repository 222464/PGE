#pragma once

#include "../scene/Scene.h"

#include "../constructs/Vec2f.h"

#include "../sceneobjects/physics/SceneObjectPhysicsWorld.h"

namespace pge {
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

	class DynamicCharacterController {
		SceneObjectRef physicsWorld;

		Scene* pScene;

		std::shared_ptr<btCollisionShape> pCollisionShape;
		std::shared_ptr<btDefaultMotionState> pMotionState;
		std::shared_ptr<btRigidBody> pRigidBody;
		std::shared_ptr<btPairCachingGhostObject> pGhostObject;

		bool onGround;
		bool hittingWall;

		float bottomYOffset;
		float bottomRoundedRegionYOffset;

		btTransform motionTransform;

		Vec3f manualVelocity;

		btVector3 previousPosition;

		float jumpRechargeTimer;

		bool mustCrouchFlag;

		Vec3f floorNormal;

		void parseGhostContacts(std::vector<Vec3f> &surfaceHitNormals);

		void updateVelocity(const std::vector<Vec3f> &surfaceHitNormals, float dt);
		void updatePosition(float dt);

	public:
		float deceleration;
		float maxSpeed;
		float jumpImpulse;

		float jumpRechargeTime;

		float stepHeight;

		DynamicCharacterController(Scene* pScene, SceneObjectPhysicsWorld* pPhysicsWorld, const Vec3f &spawnPos, float radius, float height, float mass, float stepHeight);
		~DynamicCharacterController();

		// Acceleration vector in XZ plane
		void walk(const Vec2f &dir);

		// Flattens vector
		void walk(const Vec3f &dir) {
			walk(Vec2f(dir.x, dir.z).normalized() * dir.magnitude());
		}

		void update(float dt);

		void jump();

		Vec3f getPosition() const {
			return cons(pRigidBody->getWorldTransform().getOrigin());
		}

		void setPosition(const Vec3f &position) {
			pRigidBody->getWorldTransform().setOrigin(bt(position));
		}

		Vec3f getLinearVelocity() const {
			return cons(pRigidBody->getLinearVelocity());
		}

		void setLinearVelocity(const Vec3f &velocity) {
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
	};
}