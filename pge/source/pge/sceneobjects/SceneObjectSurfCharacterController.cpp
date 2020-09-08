#include "SceneObjectSurfCharacterController.h"

#include "../scene/RenderScene.h"

#include "../util/Math.h"

#include "input/SceneObjectBufferedInput.h"

SceneObjectSurfCharacterController::SceneObjectSurfCharacterController()
: deceleration(0.1f), maxSpeed(5.0f), jumpImpulse(400.0f),
manualVelocity(0.0f, 0.0f, 0.0f), onGround(false), hittingWall(false),
jumpRechargeTimer(0.0f), jumpRechargeTime(0.1f),
mustCrouchFlag(false), floorNormal(0.0f, 1.0f, 0.0f),
sensitivity(0.01f), angleX(0.0f), angleY(0.0f)
{}

void SceneObjectSurfCharacterController::create(pge::SceneObjectPhysicsWorld* pPhysicsWorld, const pge::Vec3f &spawnPos, float radius, float height, float mass, float stepHeight) {
	physicsWorld = pPhysicsWorld;
	bottomYOffset = height * 0.5f + radius;
	bottomRoundedRegionYOffset = (height + radius) * 0.5f;
	this->stepHeight = stepHeight;

	pCollisionShape.reset(new btCapsuleShape(radius, height));

	pMotionState.reset(new btDefaultMotionState(btTransform(btQuaternion(1.0f, 0.0f, 0.0f, 0.0f).normalized(), bt(spawnPos))));

	btVector3 intertia;
	pCollisionShape->calculateLocalInertia(mass, intertia);

	btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(mass, pMotionState.get(), pCollisionShape.get(), intertia);

	// No friction, this is done manually
	rigidBodyCI.m_friction = 0.0f;
	rigidBodyCI.m_restitution = 0.0f;
	rigidBodyCI.m_linearDamping = 0.0f;

	pRigidBody.reset(new btRigidBody(rigidBodyCI));

	// Keep upright
	pRigidBody->setAngularFactor(0.0f);

	// No sleeping (or else setLinearVelocity won't work)
	pRigidBody->setActivationState(DISABLE_DEACTIVATION);

	pPhysicsWorld->pDynamicsWorld->addRigidBody(pRigidBody.get());

	pRigidBody->setUserPointer(nullptr);

	// Ghost object that is synchronized with rigid body
	pGhostObject.reset(new btPairCachingGhostObject());

	pGhostObject->setCollisionShape(pCollisionShape.get());
	pGhostObject->setUserPointer(this);
	pGhostObject->setCollisionFlags(btCollisionObject::CF_NO_CONTACT_RESPONSE);

	// Specify filters manually, otherwise ghost doesn't collide with statics for some reason
	pPhysicsWorld->pDynamicsWorld->addCollisionObject(pGhostObject.get(), btBroadphaseProxy::KinematicFilter, btBroadphaseProxy::StaticFilter | btBroadphaseProxy::DefaultFilter);

	pGhostObject->setUserPointer(nullptr);
}

SceneObjectSurfCharacterController::~SceneObjectSurfCharacterController() {
	if (physicsWorld.isAlive()) {
		pge::SceneObjectPhysicsWorld* pPhysicsWorld = static_cast<pge::SceneObjectPhysicsWorld*>(physicsWorld.get());

		pPhysicsWorld->pDynamicsWorld->removeRigidBody(pRigidBody.get());
		pPhysicsWorld->pDynamicsWorld->removeCollisionObject(pGhostObject.get());
	}
}

void SceneObjectSurfCharacterController::walk(const pge::Vec2f &dir) {
	// Prevent from going over maximum speed
	float speed = manualVelocity.magnitude();

	if (speed < maxSpeed)
		manualVelocity += pge::Vec3f(dir.x, 0.0f, dir.y);
}

void SceneObjectSurfCharacterController::physUpdate(float dt) {
	// Sync ghost with actually object
	pGhostObject->setWorldTransform(pRigidBody->getWorldTransform());

	// Update transform
	pMotionState->getWorldTransform(motionTransform);

	onGround = false;
	floorNormal = pge::Vec3f(0.0f, 1.0f, 0.0f);

	std::vector<pge::Vec3f> surfaceHitNormals;

	parseGhostContacts(surfaceHitNormals);

	updatePosition(dt);
	updateVelocity(surfaceHitNormals, dt);

	// Update jump timer
	if (jumpRechargeTimer < jumpRechargeTime)
		jumpRechargeTimer += dt;
}

void SceneObjectSurfCharacterController::parseGhostContacts(std::vector<pge::Vec3f> &surfaceHitNormals) {
	btManifoldArray manifoldArray;
	btBroadphasePairArray &pairArray = pGhostObject->getOverlappingPairCache()->getOverlappingPairArray();
	int numPairs = pairArray.size();

	// Set false now, may be set true in test
	hittingWall = false;

	pge::SceneObjectPhysicsWorld* pPhysicsWorld = static_cast<pge::SceneObjectPhysicsWorld*>(physicsWorld.get());

	for (int i = 0; i < numPairs; i++) {
		manifoldArray.clear();

		const btBroadphasePair &pair = pairArray[i];

		btBroadphasePair* collisionPair = pPhysicsWorld->pDynamicsWorld->getPairCache()->findPair(pair.m_pProxy0, pair.m_pProxy1);

		if (collisionPair == nullptr)
			continue;

		if (collisionPair->m_algorithm != nullptr)
			collisionPair->m_algorithm->getAllContactManifolds(manifoldArray);

		for (int j = 0; j < manifoldArray.size(); j++) {
			btPersistentManifold* pManifold = manifoldArray[j];

			// Skip the rigid body the ghost monitors
			if (pManifold->getBody0() == pRigidBody.get())
				continue;

			for (int p = 0; p < pManifold->getNumContacts(); p++) {
				const btManifoldPoint &point = pManifold->getContactPoint(p);

				if (point.getDistance() < 0.0f) {
					const btVector3 &ptB = point.getPositionWorldOnB();

					// If point is in rounded bottom region of capsule shape, it is on the ground
					if (ptB.getY() < motionTransform.getOrigin().getY() - bottomRoundedRegionYOffset)
						onGround = true;
					else {
						hittingWall = true;

						surfaceHitNormals.push_back(cons(point.m_normalWorldOnB));
					}
				}
			}
		}
	}
}

void SceneObjectSurfCharacterController::updateVelocity(const std::vector<pge::Vec3f> &surfaceHitNormals, float dt) {
	pge::SceneObjectPhysicsWorld* pPhysicsWorld = static_cast<pge::SceneObjectPhysicsWorld*>(physicsWorld.get());

	// Adjust only xz velocity
	manualVelocity.y = pRigidBody->getLinearVelocity().getY();

	pRigidBody->setLinearVelocity(bt(manualVelocity));

	// Decelerate
	if (onGround) {
		manualVelocity.x -= manualVelocity.x * deceleration * dt;
		manualVelocity.z -= manualVelocity.z * deceleration * dt;
	}

	if (hittingWall) {
		for (size_t i = 0, size = surfaceHitNormals.size(); i < size; i++) {
			// Cancel velocity across normal
			pge::Vec3f velInNormalDir(manualVelocity.project(surfaceHitNormals[i]));

			// Apply correction
			manualVelocity -= velInNormalDir * 1.05f;
		}

		// Do not adjust rigid body velocity manually (so bodies can still be pushed by character)
		return;
	}
}

void SceneObjectSurfCharacterController::updatePosition(float dt) {
	mustCrouchFlag = false;

	pge::SceneObjectPhysicsWorld* pPhysicsWorld = static_cast<pge::SceneObjectPhysicsWorld*>(physicsWorld.get());

	// Ray cast, ignore rigid body
	IgnoreBodyAndGhostCast rayCallBackBottom(pRigidBody.get(), pGhostObject.get());

	pPhysicsWorld->pDynamicsWorld->rayTest(pRigidBody->getWorldTransform().getOrigin(), pRigidBody->getWorldTransform().getOrigin() - btVector3(0.0f, bottomYOffset + stepHeight, 0.0f), rayCallBackBottom);

	// Bump up if hit
	if (rayCallBackBottom.hasHit()) {
		float previousY = pRigidBody->getWorldTransform().getOrigin().getY();

		if (pRigidBody->getLinearVelocity().getY() < 0.01f) {
			pRigidBody->getWorldTransform().getOrigin().setY(previousY + (bottomYOffset + stepHeight) * (1.0f - rayCallBackBottom.m_closestHitFraction) * 0.5f * dt);

			pge::Vec3f vel(cons(pRigidBody->getLinearVelocity()));

			// Cancel velocity across normal
			floorNormal = cons(rayCallBackBottom.m_hitNormalWorld);

			pge::Vec3f velInNormalDir(vel.project(floorNormal));

			// Apply correction
			manualVelocity -= velInNormalDir * 1.05f;

			pRigidBody->setLinearVelocity(bt(manualVelocity));
		}

		onGround = true;
	}

	float testOffset = 0.01f;

	// Ray cast, ignore rigid body
	IgnoreBodyAndGhostCast rayCallBackTop(pRigidBody.get(), pGhostObject.get());

	pPhysicsWorld->pDynamicsWorld->rayTest(pRigidBody->getWorldTransform().getOrigin(), pRigidBody->getWorldTransform().getOrigin() + btVector3(0.0f, bottomYOffset + testOffset, 0.0f), rayCallBackTop);

	// Bump down if hit
	if (rayCallBackTop.hasHit()) {
		pRigidBody->getWorldTransform().setOrigin(previousPosition);

		pge::Vec3f vel(cons(pRigidBody->getLinearVelocity()));

		// Cancel velocity across normal
		pge::Vec3f velInNormalDir(vel.project(cons(rayCallBackTop.m_hitNormalWorld)));

		// Apply correction
		manualVelocity -= velInNormalDir * 1.05f;

		pRigidBody->setLinearVelocity(bt(manualVelocity));

		if (onGround)
			mustCrouchFlag = true;
	}

	previousPosition = pRigidBody->getWorldTransform().getOrigin();
}

void SceneObjectSurfCharacterController::jump() {
	if (onGround && jumpRechargeTimer >= jumpRechargeTime) {
		jumpRechargeTimer = 0.0f;
		pRigidBody->applyCentralImpulse(btVector3(0.0f, jumpImpulse, 0.0f));

		// Move upwards slightly so velocity isn't immediately cancelled when it detects it as on ground next frame
		const float jumpYOffset = 0.01f;

		float previousY = pRigidBody->getWorldTransform().getOrigin().getY();

		pRigidBody->getWorldTransform().getOrigin().setY(previousY + jumpYOffset);
	}
}

void SceneObjectSurfCharacterController::onAdd() {
	input = getScene()->getNamed("buffIn");
}

void SceneObjectSurfCharacterController::update(float dt) {
	pge::SceneObjectBufferedInput* pBufferedInput = static_cast<pge::SceneObjectBufferedInput*>(input.get());

	sf::Vector2i mousePosition = sf::Mouse::getPosition(*getRenderScene()->getRenderWindow()) - sf::Vector2i(128, 128);
	sf::Mouse::setPosition(sf::Vector2i(128, 128), *getRenderScene()->getRenderWindow());

	angleX -= mousePosition.x * sensitivity;
	angleY -= mousePosition.y * sensitivity;

	angleX = fmodf(angleX, pge::piTimes2);

	if (angleY < -pge::piOver2)
		angleY = -pge::piOver2;
	else if (angleY > pge::piOver2)
		angleY = pge::piOver2;

	getRenderScene()->logicCamera.rotation = pge::Quaternion(angleX, pge::Vec3f(0.0f, 1.0f, 0.0f)) * pge::Quaternion(angleY, pge::Vec3f(1.0f, 0.0f, 0.0f));

	getRenderScene()->logicCamera.position = getPosition() + pge::Vec3f(0.0f, bottomYOffset, 0.0f);

	physUpdate(dt);
}
