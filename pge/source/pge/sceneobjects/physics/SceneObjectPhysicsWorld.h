#pragma once

#include "../../scene/Scene.h"

#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>

#include "../../physics/BulletConversions.h"

#include <BulletDynamics/ConstraintSolver/btNNCGConstraintSolver.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>

namespace pge {
	class SceneObjectPhysicsWorld : public pge::SceneObject {
	public:
		int steps;
		int subSteps;

		std::shared_ptr<btBroadphaseInterface> pBroadphase;
		std::shared_ptr<btDefaultCollisionConfiguration> pCollisionConfiguration;
		std::shared_ptr<btCollisionDispatcher> pDispatcher;

		std::shared_ptr<btNNCGConstraintSolver> pSolver;

		std::shared_ptr<btDiscreteDynamicsWorld> pDynamicsWorld;

		std::shared_ptr<btGhostPairCallback> pGhostPairCallBack;

		SceneObjectPhysicsWorld();

		// Inherited from SceneObject
		void onQueue();
		void onDestroy();
		void synchronousUpdate(float dt);

		SceneObject* copyFactory() {
			return new SceneObjectPhysicsWorld(*this);
		}
	};
}