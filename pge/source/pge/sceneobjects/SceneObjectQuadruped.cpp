#include <pge/sceneobjects/SceneObjectQuadruped.h>

#include <pge/rendering/model/SceneObjectStaticModelBatcher.h>

#include <pge/sceneobjects/SceneObjectOrbitCamera.h>

#include <pge/util/Math.h>

#include <iostream>
#include <sstream>

const int constraintSolverIterations = 80;
const float maxLimbBend = pge::_pi * 0.125f;
const float maxSpeed = 30.0f;
const float maxForce = 5000.0f;
const float interpFactor = 400.0f;

void SceneObjectQuadruped::Leg::create(pge::SceneObjectPhysicsWorld* pPhysicsWorld, btRigidBody* pBodyPart, const btVector3 &rootPos) {
	// Remove old
	if (_lower._pConstraint != nullptr)
		pPhysicsWorld->_pDynamicsWorld->removeConstraint(_lower._pConstraint.get());

	if (_lower._pRigidBody != nullptr)
		pPhysicsWorld->_pDynamicsWorld->removeRigidBody(_lower._pRigidBody.get());

	if (_upper._pConstraint != nullptr)
		pPhysicsWorld->_pDynamicsWorld->removeConstraint(_upper._pConstraint.get());

	if (_upper._pRigidBody != nullptr)
		pPhysicsWorld->_pDynamicsWorld->removeRigidBody(_upper._pRigidBody.get());

	if (_pGhostLower != nullptr)
		pPhysicsWorld->_pDynamicsWorld->removeCollisionObject(_pGhostLower.get());

	// Physics
	const float height = 0.5f;
	const float radius = 0.12f;

	_lower._pCollisionShape.reset(new btCapsuleShape(radius, height));
	_upper._pCollisionShape.reset(new btCapsuleShape(radius, height));

	_lower._pMotionState.reset(new btDefaultMotionState(btTransform(btQuaternion(btVector3(0.0f, 0.0f, 1.0f), pge::_piOver2), rootPos + btVector3(-0.5f * 0.25f, 0.5f * 0.25f, 0.0f))));
	_upper._pMotionState.reset(new btDefaultMotionState(btTransform(btQuaternion(btVector3(0.0f, 0.0f, 1.0f), -pge::_piOver2), rootPos + btVector3(0.5f * 0.25f, 0.5f * 0.25f + 0.75f, 0.0f))));

	const float lowerMass = 6.0f;
	const float upperMass = 6.0f;

	btVector3 lowerInertia, upperInertia;

	_lower._pCollisionShape->calculateLocalInertia(lowerMass, lowerInertia);
	_upper._pCollisionShape->calculateLocalInertia(upperMass, upperInertia);

	btRigidBody::btRigidBodyConstructionInfo rigidBodyCILower(lowerMass, _lower._pMotionState.get(), _lower._pCollisionShape.get(), lowerInertia);
	btRigidBody::btRigidBodyConstructionInfo rigidBodyCIUpper(upperMass, _upper._pMotionState.get(), _upper._pCollisionShape.get(), upperInertia);

	rigidBodyCILower.m_restitution = 0.05f;
	rigidBodyCILower.m_friction = 10.0f;

	rigidBodyCIUpper.m_restitution = 0.05f;
	rigidBodyCIUpper.m_friction = 0.5f;

	_lower._pRigidBody.reset(new btRigidBody(rigidBodyCILower));
	_upper._pRigidBody.reset(new btRigidBody(rigidBodyCIUpper));

	_lower._pRigidBody->setActivationState(DISABLE_DEACTIVATION);
	_upper._pRigidBody->setActivationState(DISABLE_DEACTIVATION);

	pPhysicsWorld->_pDynamicsWorld->addRigidBody(_lower._pRigidBody.get(), 1 << 2, 0x0001);
	pPhysicsWorld->_pDynamicsWorld->addRigidBody(_upper._pRigidBody.get(), 1 << 3, 0x0001);

	btTransform frameLowerUpper = btTransform::getIdentity();
	btTransform frameUpperLower = btTransform::getIdentity();
	btTransform frameUpperBody = btTransform::getIdentity();
	btTransform frameBodyUpper = btTransform::getIdentity();

	frameLowerUpper.setOrigin(btVector3(0.0f, 0.25f, 0.0f));
	frameLowerUpper.setRotation(btQuaternion(btVector3(0.0f, 0.0f, 1.0f), -pge::_piOver2));

	frameUpperLower.setOrigin(btVector3(0.0f, -0.25f, 0.0f));

	frameUpperBody.setOrigin(btVector3(0.0f, 0.25f, 0.0f));
	frameUpperBody.setRotation(btQuaternion(btVector3(0.0f, 0.0f, 1.0f), pge::_piOver4));

	frameBodyUpper.setOrigin(rootPos);
	
	_lower._pConstraint.reset(new btGeneric6DofConstraint(*_lower._pRigidBody, *_upper._pRigidBody, frameLowerUpper, frameUpperLower, false));
	_upper._pConstraint.reset(new btGeneric6DofConstraint(*_upper._pRigidBody, *pBodyPart, frameUpperBody, frameBodyUpper, false));

	pPhysicsWorld->_pDynamicsWorld->addConstraint(_lower._pConstraint.get(), true);
	pPhysicsWorld->_pDynamicsWorld->addConstraint(_upper._pConstraint.get(), true);

	_lower._pConstraint->setLimit(0, 0.0f, 0.0f);
	_lower._pConstraint->setLimit(1, 0.0f, 0.0f);
	_lower._pConstraint->setLimit(2, 0.0f, 0.0f);

	_upper._pConstraint->setLimit(0, 0.0f, 0.0f);
	_upper._pConstraint->setLimit(1, 0.0f, 0.0f);
	_upper._pConstraint->setLimit(2, 0.0f, 0.0f);

	_lower._pConstraint->getRotationalLimitMotor(0)->m_enableMotor = true;
	_lower._pConstraint->getRotationalLimitMotor(0)->m_maxMotorForce = maxForce;
	_lower._pConstraint->getRotationalLimitMotor(1)->m_enableMotor = true;
	_lower._pConstraint->getRotationalLimitMotor(1)->m_maxMotorForce = maxForce;
	_lower._pConstraint->getRotationalLimitMotor(2)->m_enableMotor = true;
	_lower._pConstraint->getRotationalLimitMotor(2)->m_maxMotorForce = maxForce;

	_upper._pConstraint->getRotationalLimitMotor(0)->m_enableMotor = true;
	_upper._pConstraint->getRotationalLimitMotor(0)->m_maxMotorForce = maxForce;
	_upper._pConstraint->getRotationalLimitMotor(1)->m_enableMotor = true;
	_upper._pConstraint->getRotationalLimitMotor(1)->m_maxMotorForce = maxForce;
	_upper._pConstraint->getRotationalLimitMotor(2)->m_enableMotor = true;
	_upper._pConstraint->getRotationalLimitMotor(2)->m_maxMotorForce = maxForce;

	_lower._pConstraint->setOverrideNumSolverIterations(constraintSolverIterations);
	_upper._pConstraint->setOverrideNumSolverIterations(constraintSolverIterations);

	_pGhostLower.reset(new btGhostObject());

	_pGhostLower->setCollisionShape(_lower._pCollisionShape.get());
	_pGhostLower->setCollisionFlags(btCollisionObject::CF_NO_CONTACT_RESPONSE);

	pPhysicsWorld->_pDynamicsWorld->addCollisionObject(_pGhostLower.get());
}

bool SceneObjectQuadruped::create() {
	assert(getScene() != nullptr);

	// Rendering
	std::shared_ptr<pge::Asset> asset;

	if (!getScene()->getAssetManager("MOBJ", pge::StaticModelOBJ::assetFactory)->getAsset("resources/models/limb1.obj", asset))
		return false;

	_pLimbModel = static_cast<pge::StaticModelOBJ*>(asset.get());

	if (!getScene()->getAssetManager("MOBJ", pge::StaticModelOBJ::assetFactory)->getAsset("resources/models/body.obj", asset))
		return false;

	_pBodyPartModel = static_cast<pge::StaticModelOBJ*>(asset.get());

	// Get reference to physics world
	_physicsWorld = getScene()->getNamedCheckQueue("physWrld");

	_orbCam = getScene()->getNamedCheckQueue("orbCam");

	reset();

	_capture = false;

	_capBytes = std::make_shared<std::vector<char>>(getRenderScene()->_gBuffer.getWidth() * getRenderScene()->_gBuffer.getHeight() * 3, 0);

	_show = getRenderScene()->_renderingEnabled;

	_socket = std::make_shared<sf::TcpSocket>();

	_socket->connect(sf::IpAddress::LocalHost, _port, sf::seconds(5.0f));

	_doneLastFrame = false;

	_action.fill(0.0f);

	return true;
}

void SceneObjectQuadruped::onAdd() {
	_batcherRef = getScene()->getNamed("smb");

	assert(_batcherRef.isAlive());
}

void SceneObjectQuadruped::reset() {
	// Slightly random angle
	std::uniform_real_distribution<float> pertDist(-0.05f, 0.05f);

	assert(_physicsWorld.isAlive());

	pge::SceneObjectPhysicsWorld* pPhysicsWorld = static_cast<pge::SceneObjectPhysicsWorld*>(_physicsWorld.get());

	// Remove old
	if (_pConstraintForwardBackward != nullptr)
		pPhysicsWorld->_pDynamicsWorld->removeConstraint(_pConstraintForwardBackward.get());

	if (_pRigidBodyFloor != nullptr)
		pPhysicsWorld->_pDynamicsWorld->removeRigidBody(_pRigidBodyFloor.get());

	if (_pRigidBodyBackward != nullptr)
		pPhysicsWorld->_pDynamicsWorld->removeRigidBody(_pRigidBodyBackward.get());

	if (_pRigidBodyForward != nullptr)
		pPhysicsWorld->_pDynamicsWorld->removeRigidBody(_pRigidBodyForward.get());

	// Physics
	const btVector3 bodyPartSize(0.5f, 0.25f, 0.5f);

	_pCollisionShapeFloor.reset(new btBoxShape(btVector3(1000.0f, 0.5f, 1000.0f)));
	_pCollisionShapeBodyForward.reset(new btBoxShape(bodyPartSize));
	_pCollisionShapeBodyBackward.reset(new btBoxShape(bodyPartSize));
	
	_pMotionStateFloor.reset(new btDefaultMotionState(btTransform(btQuaternion(0.0f, 0.0f, 0.0f), btVector3(0.0f, -0.5f, 0.0f))));
	_pMotionStateBodyForward.reset(new btDefaultMotionState(btTransform(btQuaternion(0.0f, 0.0f, 0.0f), btVector3(0.0f, 1.0f, 0.0f))));
	_pMotionStateBodyBackward.reset(new btDefaultMotionState(btTransform(btQuaternion(0.0f, 0.0f, 0.0f), btVector3(0.0f, 1.0f, 0.0f))));
	
	const float floorMass = 0.0f;
	const float forwardMass = 20.0f;
	const float backwardMass = 20.0f;

	btVector3 floorInertia, forwardInertia, backwardInertia;

	_pCollisionShapeBodyForward->calculateLocalInertia(floorMass, floorInertia);
	_pCollisionShapeBodyForward->calculateLocalInertia(forwardMass, forwardInertia);
	_pCollisionShapeBodyBackward->calculateLocalInertia(backwardMass, backwardInertia);

	btRigidBody::btRigidBodyConstructionInfo rigidBodyCIFloor(floorMass, _pMotionStateFloor.get(), _pCollisionShapeFloor.get(), floorInertia);
	btRigidBody::btRigidBodyConstructionInfo rigidBodyCIForward(forwardMass, _pMotionStateBodyForward.get(), _pCollisionShapeBodyForward.get(), forwardInertia);
	btRigidBody::btRigidBodyConstructionInfo rigidBodyCIBackward(backwardMass, _pMotionStateBodyBackward.get(), _pCollisionShapeBodyBackward.get(), backwardInertia);

	rigidBodyCIFloor.m_restitution = 0.05f;
	rigidBodyCIFloor.m_friction = 0.5f;

	rigidBodyCIForward.m_restitution = 0.05f;
	rigidBodyCIForward.m_friction = 0.5f;

	rigidBodyCIBackward.m_restitution = 0.05f;
	rigidBodyCIBackward.m_friction = 0.5f;

	_pRigidBodyFloor.reset(new btRigidBody(rigidBodyCIFloor));
	_pRigidBodyForward.reset(new btRigidBody(rigidBodyCIForward));
	_pRigidBodyBackward.reset(new btRigidBody(rigidBodyCIBackward));

	_pRigidBodyForward->setActivationState(DISABLE_DEACTIVATION);
	_pRigidBodyBackward->setActivationState(DISABLE_DEACTIVATION);

	pPhysicsWorld->_pDynamicsWorld->addRigidBody(_pRigidBodyFloor.get(), 0xffff, 0xffff);
	pPhysicsWorld->_pDynamicsWorld->addRigidBody(_pRigidBodyForward.get(), 1 << 4, 0x0001);
	pPhysicsWorld->_pDynamicsWorld->addRigidBody(_pRigidBodyBackward.get(), 1 << 5, 0x0001);

	btTransform frameA = btTransform::getIdentity();
	btTransform frameB = btTransform::getIdentity();

	frameA.setOrigin(btVector3(-0.5f, 0.0f, 0.0f));
	frameB.setOrigin(btVector3(0.5f, 0.0f, 0.0f));
	_pConstraintForwardBackward.reset(new btGeneric6DofConstraint(*_pRigidBodyForward, *_pRigidBodyBackward, frameA, frameB, false));

	pPhysicsWorld->_pDynamicsWorld->addConstraint(_pConstraintForwardBackward.get(), true);

	_pConstraintForwardBackward->setLimit(0, 0.0f, 0.0f);
	_pConstraintForwardBackward->setLimit(1, 0.0f, 0.0f);
	_pConstraintForwardBackward->setLimit(2, 0.0f, 0.0f);

	_pConstraintForwardBackward->getRotationalLimitMotor(0)->m_enableMotor = true;
	_pConstraintForwardBackward->getRotationalLimitMotor(0)->m_maxMotorForce = maxForce;
	_pConstraintForwardBackward->getRotationalLimitMotor(1)->m_enableMotor = true;
	_pConstraintForwardBackward->getRotationalLimitMotor(1)->m_maxMotorForce = maxForce;
	_pConstraintForwardBackward->getRotationalLimitMotor(2)->m_enableMotor = true;
	_pConstraintForwardBackward->getRotationalLimitMotor(2)->m_maxMotorForce = maxForce;

	_pConstraintForwardBackward->setOverrideNumSolverIterations(constraintSolverIterations);

	// Create limbs
	_legs[0].create(pPhysicsWorld, _pRigidBodyBackward.get(), btVector3(0.0f, 0.0f, -0.333f));
	_legs[1].create(pPhysicsWorld, _pRigidBodyForward.get(), btVector3(0.0f, 0.0f, -0.333f));
	_legs[2].create(pPhysicsWorld, _pRigidBodyForward.get(), btVector3(0.0f, 0.0f, 0.333f));
	_legs[3].create(pPhysicsWorld, _pRigidBodyBackward.get(), btVector3(0.0f, 0.0f, 0.333f));

	_ticksPerAction = 0;
	_ticks = 0;
	_reward = 0.0f;
	_prevDist = 0.0f;
}

void SceneObjectQuadruped::act(float dt) {
	// Set to target angles
	int actIndex = 0;

	for (int i = 0; i < 4; i++) {
		_legs[i]._lower._pConstraint->getRotationalLimitMotor(0)->m_targetVelocity = std::min(maxSpeed, std::max(-maxSpeed, dt * interpFactor * (_action[actIndex++] * maxLimbBend - _legs[i]._lower._pConstraint->getAngle(0))));
		_legs[i]._lower._pConstraint->getRotationalLimitMotor(1)->m_targetVelocity = std::min(maxSpeed, std::max(-maxSpeed, dt * interpFactor * (_action[actIndex++] * maxLimbBend - _legs[i]._lower._pConstraint->getAngle(1))));
		_legs[i]._lower._pConstraint->getRotationalLimitMotor(2)->m_targetVelocity = std::min(maxSpeed, std::max(-maxSpeed, dt * interpFactor * (_action[actIndex++] * maxLimbBend - _legs[i]._lower._pConstraint->getAngle(2))));
	
		_legs[i]._upper._pConstraint->getRotationalLimitMotor(0)->m_targetVelocity = std::min(maxSpeed, std::max(-maxSpeed, dt * interpFactor * (_action[actIndex++] * maxLimbBend - _legs[i]._upper._pConstraint->getAngle(0))));
		_legs[i]._upper._pConstraint->getRotationalLimitMotor(1)->m_targetVelocity = std::min(maxSpeed, std::max(-maxSpeed, dt * interpFactor * (_action[actIndex++] * maxLimbBend - _legs[i]._upper._pConstraint->getAngle(1))));
		_legs[i]._upper._pConstraint->getRotationalLimitMotor(2)->m_targetVelocity = std::min(maxSpeed, std::max(-maxSpeed, dt * interpFactor * (_action[actIndex++] * maxLimbBend - _legs[i]._upper._pConstraint->getAngle(2))));
	}

	_pConstraintForwardBackward->getRotationalLimitMotor(0)->m_targetVelocity = std::min(maxSpeed, std::max(-maxSpeed, dt * interpFactor * (_action[actIndex++] * maxLimbBend - _pConstraintForwardBackward->getAngle(0))));
	_pConstraintForwardBackward->getRotationalLimitMotor(1)->m_targetVelocity = std::min(maxSpeed, std::max(-maxSpeed, dt * interpFactor * (_action[actIndex++] * maxLimbBend - _pConstraintForwardBackward->getAngle(1))));
	_pConstraintForwardBackward->getRotationalLimitMotor(2)->m_targetVelocity = std::min(maxSpeed, std::max(-maxSpeed, dt * interpFactor * (_action[actIndex++] * maxLimbBend - _pConstraintForwardBackward->getAngle(2))));

	float dist = getPosition().magnitude();

	_reward = dist - _prevDist;

	// If fell over
	if (_pRigidBodyForward->getWorldTransform().getRotation().angleShortestPath(btQuaternion::getIdentity()) > pge::_piOver2 * 0.9f) {
		_doneLastFrame = true;

		_reward = -1.0f;

		reset();
	}

	_prevDist = dist;
}

void SceneObjectQuadruped::synchronousUpdate(float dt) {
	if (_ticks >= _ticksPerAction || !getRenderScene()->_renderingEnabled) {
		_ticks = 0;

		std::array<char, _maxBatchSize> buffer;

		std::array<char, 1 + 4 * 27> msg;

		size_t received = 0;
		size_t totalReceived = 0;

		while (totalReceived < msg.size()) {
			_socket->receive(buffer.data(), msg.size() - totalReceived, received);

			for (int i = 0; i < received; i++)
				msg[totalReceived + i] = buffer[i];

			totalReceived += received;
		}

		if (msg[0] == 'A') { // Action
			for (int i = 0; i < 27; i++) {
				_action[i] = *reinterpret_cast<float*>(&msg[1 + 4 * i]);
			}
		}
		else if (msg[0] == 'R') { // Reset
			for (int i = 0; i < 27; i++) {
				_action[i] = *reinterpret_cast<float*>(&msg[1 + 4 * i]);
			}
			reset();
		}
		else if (msg[0] == 'C') { // Capture + action
			for (int i = 0; i < 27; i++) {
				_action[i] = *reinterpret_cast<float*>(&msg[1 + 4 * i]);
			}
			_capture = true;

			if (!getRenderScene()->_renderingEnabled) {
				getRenderScene()->getRenderWindow()->setFramerateLimit(60);
				getRenderScene()->getRenderWindow()->setVerticalSyncEnabled(true);
			}

			getRenderScene()->_renderingEnabled = true;
		}
		else if (msg[0] == 'S') { // Stop capture + action
			for (int i = 0; i < 27; i++) {
				_action[i] = *reinterpret_cast<float*>(&msg[1 + 4 * i]);
			}
			_capture = false;

			if (!_show) {
				if (getRenderScene()->_renderingEnabled) {
					getRenderScene()->getRenderWindow()->setFramerateLimit(0);
					getRenderScene()->getRenderWindow()->setVerticalSyncEnabled(false);
				}

				getRenderScene()->_renderingEnabled = false;
			}
		}
		else if (msg[0] == 'X') { // Exit
			getRenderScene()->_close = true;
		}

		act(dt);

		// Give state and reward (+ capture if is on)

		// Observation (34 values)
		std::vector<float> obs(34);

		int obsIndex = 0;

		for (int i = 0; i < 4; i++) {
			pge::Vec3f eulerLower = cons(_legs[i]._lower._pConstraint->getRigidBodyB().getWorldTransform().getRotation().inverse() * _legs[i]._lower._pConstraint->getRigidBodyA().getWorldTransform().getRotation()).getEulerAngles();
			pge::Vec3f eulerUpper = cons(_legs[i]._upper._pConstraint->getRigidBodyB().getWorldTransform().getRotation().inverse() * _legs[i]._upper._pConstraint->getRigidBodyA().getWorldTransform().getRotation()).getEulerAngles();

			obs[obsIndex++] = eulerLower.x;
			obs[obsIndex++] = eulerLower.y;
			obs[obsIndex++] = eulerLower.z;

			obs[obsIndex++] = eulerUpper.x;
			obs[obsIndex++] = eulerUpper.y;
			obs[obsIndex++] = eulerUpper.z;
		}

		pge::Vec3f eulerBody = cons(_pConstraintForwardBackward->getRigidBodyB().getWorldTransform().getRotation().inverse() * _pConstraintForwardBackward->getRigidBodyA().getWorldTransform().getRotation()).getEulerAngles();

		obs[obsIndex++] = eulerBody.x;
		obs[obsIndex++] = eulerBody.y;
		obs[obsIndex++] = eulerBody.z;

		// Gravity sensor
		pge::Vec3f eulerGrav = cons(_pRigidBodyForward->getWorldTransform().getRotation()).getEulerAngles();

		obs[obsIndex++] = eulerGrav.x;
		obs[obsIndex++] = eulerGrav.y;
		obs[obsIndex++] = eulerGrav.z;

		// Touch sensors (x4)
		for (int i = 0; i < 4; i++) {
			int num = _legs[i]._pGhostLower->getNumOverlappingObjects();

			bool hit = num > 0;

			obs[obsIndex++] = hit ? 1.0f : 0.0f;
		}

		// Update ghosts
		for (int i = 0; i < 4; i++)
			_legs[i]._pGhostLower->setWorldTransform(_legs[i]._lower._pRigidBody->getWorldTransform());

		// First add reward
		int index = 0;

		*reinterpret_cast<float*>(&buffer[index]) = _reward;

		index += sizeof(float);

		for (int i = 0; i < obs.size(); i++) {
			*reinterpret_cast<float*>(&buffer[index]) = obs[i];

			index += sizeof(float);
		}

		// Reset flag
		*reinterpret_cast<int*>(&buffer[index]) = static_cast<int>(_doneLastFrame);

		_doneLastFrame = false;

		index += sizeof(int);

		// Submit number of batches of _maxBatchSize
		int numBatches = _capBytes->size() / _maxBatchSize + ((_capBytes->size() % _maxBatchSize) == 0 ? 0 : 1);

		// No batches if not capturing
		if (!_capture)
			numBatches = 0;

		*reinterpret_cast<int*>(&buffer[index]) = numBatches;

		index += sizeof(int);

		_socket->send(buffer.data(), index);

		if (_capture) {
			std::vector<char> reorganized(_capBytes->size());

			int reorgIndex = 0;

			for (int y = 0; y < getRenderScene()->_gBuffer.getHeight(); y++)
				for (int x = 0; x < getRenderScene()->_gBuffer.getWidth(); x++) {
					int start = 3 * (x + (getRenderScene()->_gBuffer.getHeight() - 1 - y) * getRenderScene()->_gBuffer.getWidth());

					reorganized[reorgIndex++] = (*_capBytes)[start + 0];
					reorganized[reorgIndex++] = (*_capBytes)[start + 1];
					reorganized[reorgIndex++] = (*_capBytes)[start + 2];
				}

			int total = 0;

			for (int i = 0; i < numBatches; i++) {
				// Submit batch
				size_t count = 0;

				for (int j = 0; j < _maxBatchSize && total < _capBytes->size(); j++) {
					buffer[j] = reorganized[total++];

					count++;
				}

				_socket->send(buffer.data(), count);
			}
		}

	}
	else
		_ticks++;

	// Update camera
	if (_orbCam.isAlive()) {
		SceneObjectOrbitCamera* pCam = static_cast<SceneObjectOrbitCamera*>(_orbCam.get());

		pCam->_focusPoint = getPosition();
	}
}

void SceneObjectQuadruped::deferredRender() {
	pge::SceneObjectStaticModelBatcher* pBatcher = static_cast<pge::SceneObjectStaticModelBatcher*>(_batcherRef.get());
	
	{
		// Render body
		pge::Matrix4x4f transform;

		_pRigidBodyForward->getWorldTransform().getOpenGLMatrix(&transform._elements[0]);

		_pBodyPartModel->render(pBatcher, transform * pge::Matrix4x4f::rotateMatrixY(pge::_pi));
	}
	
	{
		// Render body
		pge::Matrix4x4f transform;

		_pRigidBodyBackward->getWorldTransform().getOpenGLMatrix(&transform._elements[0]);

		_pBodyPartModel->render(pBatcher, transform);
	}
	
	for (int i = 0; i < 4; i++) {
		{
			// Render limb
			pge::Matrix4x4f transform;

			_legs[i]._lower._pRigidBody->getWorldTransform().getOpenGLMatrix(&transform._elements[0]);

			_pLimbModel->render(pBatcher, transform);
		}

		{
			// Render limb
			pge::Matrix4x4f transform;

			_legs[i]._upper._pRigidBody->getWorldTransform().getOpenGLMatrix(&transform._elements[0]);

			_pLimbModel->render(pBatcher, transform);
		}
	}
}

void SceneObjectQuadruped::postRender() {
	// Get data from effect buffer
	glReadBuffer(GL_FRONT);

	glReadPixels(0, 0, getRenderScene()->_gBuffer.getWidth(), getRenderScene()->_gBuffer.getHeight(), GL_RGB, GL_UNSIGNED_BYTE, _capBytes->data());
}

void SceneObjectQuadruped::onDestroy() {
	if (_socket != nullptr)
		_socket->disconnect();

	if (_physicsWorld.isAlive()) {
		pge::SceneObjectPhysicsWorld* pPhysicsWorld = static_cast<pge::SceneObjectPhysicsWorld*>(_physicsWorld.get());

		if (_pConstraintForwardBackward != nullptr)
			pPhysicsWorld->_pDynamicsWorld->removeConstraint(_pConstraintForwardBackward.get());

		if (_pRigidBodyFloor != nullptr)
			pPhysicsWorld->_pDynamicsWorld->removeRigidBody(_pRigidBodyFloor.get());

		if (_pRigidBodyBackward != nullptr)
			pPhysicsWorld->_pDynamicsWorld->removeRigidBody(_pRigidBodyBackward.get());

		if (_pRigidBodyForward != nullptr)
			pPhysicsWorld->_pDynamicsWorld->removeRigidBody(_pRigidBodyForward.get());

		for (int i = 0; i < 4; i++) {
			if (_legs[i]._lower._pConstraint != nullptr)
				pPhysicsWorld->_pDynamicsWorld->removeConstraint(_legs[i]._lower._pConstraint.get());

			if (_legs[i]._lower._pRigidBody != nullptr)
				pPhysicsWorld->_pDynamicsWorld->removeRigidBody(_legs[i]._lower._pRigidBody.get());

			if (_legs[i]._upper._pConstraint != nullptr)
				pPhysicsWorld->_pDynamicsWorld->removeConstraint(_legs[i]._upper._pConstraint.get());

			if (_legs[i]._upper._pRigidBody != nullptr)
				pPhysicsWorld->_pDynamicsWorld->removeRigidBody(_legs[i]._upper._pRigidBody.get());

			if (_legs[i]._pGhostLower != nullptr)
				pPhysicsWorld->_pDynamicsWorld->removeCollisionObject(_legs[i]._pGhostLower.get());
		}
	}
}
