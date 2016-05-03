#include <pge/physics/DynamicCharacterController.h>

using namespace pge;

DynamicCharacterController::DynamicCharacterController(Scene* pScene, SceneObjectPhysicsWorld* pPhysicsWorld, const Vec3f &spawnPos, float radius, float height, float mass, float stepHeight)
: _physicsWorld(pPhysicsWorld), _pScene(pScene), _bottomYOffset(height * 0.5f + radius), _bottomRoundedRegionYOffset((height + radius) * 0.5f),
_deceleration(0.1f), _maxSpeed(5.0f), _jumpImpulse(400.0f),
_manualVelocity(0.0f, 0.0f, 0.0f), _onGround(false), _hittingWall(false),
_jumpRechargeTimer(0.0f), _jumpRechargeTime(0.1f), _stepHeight(stepHeight),
_mustCrouch(false), _floorNormal(0.0f, 1.0f, 0.0f)
{
	_pCollisionShape.reset(new btCapsuleShape(radius, height));

	_pMotionState.reset(new btDefaultMotionState(btTransform(btQuaternion(1.0f, 0.0f, 0.0f, 0.0f).normalized(), bt(spawnPos))));

	btVector3 intertia;
	_pCollisionShape->calculateLocalInertia(mass, intertia);

	btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(mass, _pMotionState.get(), _pCollisionShape.get(), intertia);

	// No friction, this is done manually
	rigidBodyCI.m_friction = 0.0f;
	rigidBodyCI.m_restitution = 0.0f;
	rigidBodyCI.m_linearDamping = 0.0f;

	_pRigidBody.reset(new btRigidBody(rigidBodyCI));

	// Keep upright
	_pRigidBody->setAngularFactor(0.0f);

	// No sleeping (or else setLinearVelocity won't work)
	_pRigidBody->setActivationState(DISABLE_DEACTIVATION);

	pPhysicsWorld->_pDynamicsWorld->addRigidBody(_pRigidBody.get());

	_pRigidBody->setUserPointer(nullptr);

	// Ghost object that is synchronized with rigid body
	_pGhostObject.reset(new btPairCachingGhostObject());

	_pGhostObject->setCollisionShape(_pCollisionShape.get());
	_pGhostObject->setUserPointer(this);
	_pGhostObject->setCollisionFlags(btCollisionObject::CF_NO_CONTACT_RESPONSE);

	// Specify filters manually, otherwise ghost doesn't collide with statics for some reason
	pPhysicsWorld->_pDynamicsWorld->addCollisionObject(_pGhostObject.get(), btBroadphaseProxy::KinematicFilter, btBroadphaseProxy::StaticFilter | btBroadphaseProxy::DefaultFilter);

	_pGhostObject->setUserPointer(nullptr);
}

DynamicCharacterController::~DynamicCharacterController() {
	if (_physicsWorld.isAlive()) {
		SceneObjectPhysicsWorld* pPhysicsWorld = static_cast<SceneObjectPhysicsWorld*>(_physicsWorld.get());

		pPhysicsWorld->_pDynamicsWorld->removeRigidBody(_pRigidBody.get());
		pPhysicsWorld->_pDynamicsWorld->removeCollisionObject(_pGhostObject.get());
	}
}

void DynamicCharacterController::walk(const Vec2f &dir) {
	// Prevent from going over maximum speed
	float speed = _manualVelocity.magnitude();

	if (speed < _maxSpeed)
		_manualVelocity += Vec3f(dir.x, 0.0f, dir.y);
}

void DynamicCharacterController::update(float dt) {
	// Sync ghost with actually object
	_pGhostObject->setWorldTransform(_pRigidBody->getWorldTransform());

	// Update transform
	_pMotionState->getWorldTransform(_motionTransform);

	_onGround = false;
	_floorNormal = Vec3f(0.0f, 1.0f, 0.0f);

	std::vector<Vec3f> surfaceHitNormals;

	parseGhostContacts(surfaceHitNormals);

	updatePosition(dt);
	updateVelocity(surfaceHitNormals, dt);

	// Update jump timer
	if (_jumpRechargeTimer < _jumpRechargeTime)
		_jumpRechargeTimer += dt;
}

void DynamicCharacterController::parseGhostContacts(std::vector<Vec3f> &surfaceHitNormals) {
	btManifoldArray manifoldArray;
	btBroadphasePairArray &pairArray = _pGhostObject->getOverlappingPairCache()->getOverlappingPairArray();
	int numPairs = pairArray.size();

	// Set false now, may be set true in test
	_hittingWall = false;

	SceneObjectPhysicsWorld* pPhysicsWorld = static_cast<SceneObjectPhysicsWorld*>(_physicsWorld.get());

	for (int i = 0; i < numPairs; i++) {
		manifoldArray.clear();

		const btBroadphasePair &pair = pairArray[i];

		btBroadphasePair* collisionPair = pPhysicsWorld->_pDynamicsWorld->getPairCache()->findPair(pair.m_pProxy0, pair.m_pProxy1);

		if (collisionPair == nullptr)
			continue;

		if (collisionPair->m_algorithm != nullptr)
			collisionPair->m_algorithm->getAllContactManifolds(manifoldArray);

		for (int j = 0; j < manifoldArray.size(); j++) {
			btPersistentManifold* pManifold = manifoldArray[j];

			// Skip the rigid body the ghost monitors
			if (pManifold->getBody0() == _pRigidBody.get())
				continue;

			for (int p = 0; p < pManifold->getNumContacts(); p++) {
				const btManifoldPoint &point = pManifold->getContactPoint(p);

				if (point.getDistance() < 0.0f) {
					const btVector3 &ptB = point.getPositionWorldOnB();

					// If point is in rounded bottom region of capsule shape, it is on the ground
					if (ptB.getY() < _motionTransform.getOrigin().getY() - _bottomRoundedRegionYOffset)
						_onGround = true;
					else {
						_hittingWall = true;

						surfaceHitNormals.push_back(cons(point.m_normalWorldOnB));
					}
				}
			}
		}
	}
}

void DynamicCharacterController::updateVelocity(const std::vector<Vec3f> &surfaceHitNormals, float dt) {
	SceneObjectPhysicsWorld* pPhysicsWorld = static_cast<SceneObjectPhysicsWorld*>(_physicsWorld.get());

	// Adjust only xz velocity
	_manualVelocity.y = _pRigidBody->getLinearVelocity().getY();

	_pRigidBody->setLinearVelocity(bt(_manualVelocity));

	// Decelerate
	if (_onGround) {
		_manualVelocity.x -= _manualVelocity.x * _deceleration * dt;
		_manualVelocity.z -= _manualVelocity.z * _deceleration * dt;
	}

	if (_hittingWall) {
		for (size_t i = 0, size = surfaceHitNormals.size(); i < size; i++) {
			// Cancel velocity across normal
			Vec3f velInNormalDir(_manualVelocity.project(surfaceHitNormals[i]));

			// Apply correction
			_manualVelocity -= velInNormalDir * 1.05f;
		}

		// Do not adjust rigid body velocity manually (so bodies can still be pushed by character)
		return;
	}
}

void DynamicCharacterController::updatePosition(float dt) {
	_mustCrouch = false;

	SceneObjectPhysicsWorld* pPhysicsWorld = static_cast<SceneObjectPhysicsWorld*>(_physicsWorld.get());

	// Ray cast, ignore rigid body
	IgnoreBodyAndGhostCast rayCallBackBottom(_pRigidBody.get(), _pGhostObject.get());

	pPhysicsWorld->_pDynamicsWorld->rayTest(_pRigidBody->getWorldTransform().getOrigin(), _pRigidBody->getWorldTransform().getOrigin() - btVector3(0.0f, _bottomYOffset + _stepHeight, 0.0f), rayCallBackBottom);

	// Bump up if hit
	if (rayCallBackBottom.hasHit()) {
		float previousY = _pRigidBody->getWorldTransform().getOrigin().getY();

		if (_pRigidBody->getLinearVelocity().getY() < 0.01f) {
			_pRigidBody->getWorldTransform().getOrigin().setY(previousY + (_bottomYOffset + _stepHeight) * (1.0f - rayCallBackBottom.m_closestHitFraction) * 0.5f * dt);

			Vec3f vel(cons(_pRigidBody->getLinearVelocity()));

			// Cancel velocity across normal
			_floorNormal = cons(rayCallBackBottom.m_hitNormalWorld);

			Vec3f velInNormalDir(vel.project(_floorNormal));

			// Apply correction
			_manualVelocity -= velInNormalDir * 1.05f;

			_pRigidBody->setLinearVelocity(bt(_manualVelocity));
		}

		_onGround = true;
	}

	float testOffset = 0.01f;

	// Ray cast, ignore rigid body
	IgnoreBodyAndGhostCast rayCallBackTop(_pRigidBody.get(), _pGhostObject.get());

	pPhysicsWorld->_pDynamicsWorld->rayTest(_pRigidBody->getWorldTransform().getOrigin(), _pRigidBody->getWorldTransform().getOrigin() + btVector3(0.0f, _bottomYOffset + testOffset, 0.0f), rayCallBackTop);

	// Bump down if hit
	if (rayCallBackTop.hasHit()) {
		_pRigidBody->getWorldTransform().setOrigin(_previousPosition);

		Vec3f vel(cons(_pRigidBody->getLinearVelocity()));

		// Cancel velocity across normal
		Vec3f velInNormalDir(vel.project(cons(rayCallBackTop.m_hitNormalWorld)));

		// Apply correction
		_manualVelocity -= velInNormalDir * 1.05f;

		_pRigidBody->setLinearVelocity(bt(_manualVelocity));

		if (_onGround)
			_mustCrouch = true;
	}

	_previousPosition = _pRigidBody->getWorldTransform().getOrigin();
}

void DynamicCharacterController::jump() {
	if (_onGround && _jumpRechargeTimer >= _jumpRechargeTime) {
		_jumpRechargeTimer = 0.0f;
		_pRigidBody->applyCentralImpulse(btVector3(0.0f, _jumpImpulse, 0.0f));

		// Move upwards slightly so velocity isn't immediately cancelled when it detects it as on ground next frame
		const float jumpYOffset = 0.01f;

		float previousY = _pRigidBody->getWorldTransform().getOrigin().getY();

		_pRigidBody->getWorldTransform().getOrigin().setY(previousY + jumpYOffset);
	}
}