#include <pge/sceneobjects/SceneObjectCartPole.h>

#include <pge/rendering/model/SceneObjectStaticModelBatcher.h>

#include <pge/util/Math.h>

#include <iostream>
#include <sstream>

bool SceneObjectCartPole::create() {
	assert(getScene() != nullptr);

	// Rendering
	std::shared_ptr<pge::Asset> asset;

	if (!getScene()->getAssetManager("MOBJ", pge::StaticModelOBJ::assetFactory)->getAsset("resources/models/Cart.obj", asset))
		return false;

	_pCartModel = static_cast<pge::StaticModelOBJ*>(asset.get());

	if (!getScene()->getAssetManager("MOBJ", pge::StaticModelOBJ::assetFactory)->getAsset("resources/models/Pole.obj", asset))
		return false;

	_pPoleModel = static_cast<pge::StaticModelOBJ*>(asset.get());

	// Get reference to physics world
	_physicsWorld = getScene()->getNamedCheckQueue("physWrld");

	reset();

	_capture = false;

	_capBytes = std::make_shared<std::vector<char>>(getRenderScene()->_gBuffer.getWidth() * getRenderScene()->_gBuffer.getHeight() * 3, 0);

	_show = getRenderScene()->_renderingEnabled;

	_socket = std::make_shared<sf::TcpSocket>();

	_socket->connect(sf::IpAddress::LocalHost, _port, sf::seconds(5.0f));

	_doneLastFrame = false;

	return true;
}

void SceneObjectCartPole::onAdd() {
	_batcherRef = getScene()->getNamed("smb");

	assert(_batcherRef.isAlive());
}

void SceneObjectCartPole::reset() {
	// Slightly random angle
	std::uniform_real_distribution<float> pertDist(-0.05f, 0.05f);

	assert(_physicsWorld.isAlive());

	pge::SceneObjectPhysicsWorld* pPhysicsWorld = static_cast<pge::SceneObjectPhysicsWorld*>(_physicsWorld.get());

	// Remove old
	if (_pConstraint != nullptr)
		pPhysicsWorld->_pDynamicsWorld->removeConstraint(_pConstraint.get());

	if (_pRigidBodyFloor != nullptr)
		pPhysicsWorld->_pDynamicsWorld->removeRigidBody(_pRigidBodyFloor.get());

	if (_pRigidBodyCart != nullptr)
		pPhysicsWorld->_pDynamicsWorld->removeRigidBody(_pRigidBodyCart.get());

	if (_pRigidBodyPole != nullptr)
		pPhysicsWorld->_pDynamicsWorld->removeRigidBody(_pRigidBodyPole.get());

	// Physics
	_pCollisionShapeFloor.reset(new btBoxShape(btVector3(4.0f, 0.5f, 4.0f)));
	_pCollisionShapeCart.reset(new btBoxShape(bt(_pCartModel->getAABB().getHalfDims())));
	_pCollisionShapePole.reset(new btCapsuleShape(0.05f, 1.0f));

	_pMotionStateFloor.reset(new btDefaultMotionState(btTransform(btQuaternion(0.0f, 0.0f, 0.0f), btVector3(0.0f, 0.25f, 0.0f))));
	_pMotionStateCart.reset(new btDefaultMotionState(btTransform(btQuaternion(0.0f, 0.0f, 0.0f), btVector3(0.0f, 0.85f, 0.0f))));
	_pMotionStatePole.reset(new btDefaultMotionState(btTransform(btQuaternion(0.0f, pertDist(_rng), pertDist(_rng)), btVector3(0.0f, 0.85f + 0.5f + 0.05f, 0.0f))));

	const float floorMass = 0.0f;
	const float cartMass = 10.0f;
	const float poleMass = 10.0f;

	btVector3 floorInertia, cartInertia, poleInertia;

	btVector3 inertia;
	_pCollisionShapeFloor->calculateLocalInertia(floorMass, floorInertia);
	_pCollisionShapeCart->calculateLocalInertia(cartMass, cartInertia);
	_pCollisionShapePole->calculateLocalInertia(poleMass, poleInertia);

	btRigidBody::btRigidBodyConstructionInfo rigidBodyCIFloor(floorMass, _pMotionStateFloor.get(), _pCollisionShapeFloor.get(), floorInertia);
	btRigidBody::btRigidBodyConstructionInfo rigidBodyCICart(cartMass, _pMotionStateCart.get(), _pCollisionShapeCart.get(), cartInertia);
	btRigidBody::btRigidBodyConstructionInfo rigidBodyCIPole(poleMass, _pMotionStatePole.get(), _pCollisionShapePole.get(), poleInertia);

	rigidBodyCIFloor.m_restitution = 0.001f;
	rigidBodyCIFloor.m_friction = 0.01f;

	rigidBodyCICart.m_restitution = 0.001f;
	rigidBodyCICart.m_friction = 0.01f;

	rigidBodyCIPole.m_restitution = 0.001f;
	rigidBodyCIPole.m_friction = 0.01f;

	_pRigidBodyFloor.reset(new btRigidBody(rigidBodyCIFloor));
	_pRigidBodyCart.reset(new btRigidBody(rigidBodyCICart));
	_pRigidBodyPole.reset(new btRigidBody(rigidBodyCIPole));

	pPhysicsWorld->_pDynamicsWorld->addRigidBody(_pRigidBodyFloor.get());
	pPhysicsWorld->_pDynamicsWorld->addRigidBody(_pRigidBodyCart.get());
	pPhysicsWorld->_pDynamicsWorld->addRigidBody(_pRigidBodyPole.get());

	_pRigidBodyCart->setAngularFactor(0.0f); // No rotation

	btTransform frameA = btTransform::getIdentity();
	btTransform frameB = btTransform::getIdentity();

	frameA.setOrigin(btVector3(0.0f, 0.05f, 0.0f));
	frameB.setOrigin(btVector3(0.0f, -0.5f, 0.0f));
	_pConstraint.reset(new btConeTwistConstraint(*_pRigidBodyCart, *_pRigidBodyPole, frameA, frameB));

	pPhysicsWorld->_pDynamicsWorld->addConstraint(_pConstraint.get(), true);

	_action = pge::Vec2f(0.0f, 0.0f);
	_ticksPerAction = 0;
	_ticks = 0;
	_reward = 0.0f;
}

void SceneObjectCartPole::act() {
	const float force = 1000.0f;
	const float positionTolerance = 3.5f;
	const float angleTolerance = 0.7f;

	_pRigidBodyCart->applyCentralForce(btVector3(_action.x, 0.0f, _action.y) * force);

	btVector3 pos = _pRigidBodyCart->getWorldTransform().getOrigin();
	btQuaternion rot = _pRigidBodyPole->getWorldTransform().getRotation();

	_reward = -rot.angleShortestPath(btQuaternion::getIdentity());

	if (_reward < -angleTolerance ||
		std::abs(pos.getX()) > positionTolerance || std::abs(pos.getZ()) > positionTolerance) {
		std::cout << "Pole fell or is out of bounds. Resetting..." << std::endl;
		reset();
		_doneLastFrame = true;
	}
}

void SceneObjectCartPole::synchronousUpdate(float dt) {
	if (_ticks >= _ticksPerAction || !getRenderScene()->_renderingEnabled) {
		_ticks = 0;

		std::array<char, _maxBatchSize> buffer;

		std::array<char, 1 + 4 + 4> msg;

		size_t received = 0;
		size_t totalReceived = 0;

		while (totalReceived < msg.size()) {
			_socket->receive(buffer.data(), msg.size() - totalReceived, received);

			for (int i = 0; i < received; i++)
				msg[totalReceived + i] = buffer[i];

			totalReceived += received;
		}

		if (msg[0] == 'A') { // Action
			_action = pge::Vec2f(*reinterpret_cast<float*>(&msg[1]), *reinterpret_cast<float*>(&msg[5]));
		}
		else if (msg[0] == 'R') { // Reset
			_action = pge::Vec2f(*reinterpret_cast<float*>(&msg[1]), *reinterpret_cast<float*>(&msg[5]));

			reset();
		}
		else if (msg[0] == 'C') { // Capture + action
			_action = pge::Vec2f(*reinterpret_cast<float*>(&msg[1]), *reinterpret_cast<float*>(&msg[5]));

			_capture = true;

			getRenderScene()->_renderingEnabled = true;
		}
		else if (msg[0] == 'S') { // Stop capture + action
			_action = pge::Vec2f(*reinterpret_cast<float*>(&msg[1]), *reinterpret_cast<float*>(&msg[5]));

			_capture = false;

			if (!_show) {
				getRenderScene()->_renderingEnabled = false;
			}
		}
		else if (msg[0] == 'X') { // Exit
			getRenderScene()->_close = true;
		}

		_action.x = std::min(1.0f, std::max(-1.0f, _action.x));
		_action.y = std::min(1.0f, std::max(-1.0f, _action.y));
		
		act();

		// Give state and reward (+ capture if is on)

		// Observation (8 values)
		std::vector<float> obs(8);

		btVector3 pos = _pRigidBodyCart->getWorldTransform().getOrigin();
		btVector3 vel = _pRigidBodyCart->getLinearVelocity();
		btQuaternion rot = _pRigidBodyPole->getWorldTransform().getRotation();
		btVector3 angleVel = _pRigidBodyPole->getAngularVelocity();

		pge::Quaternion rotC = cons(rot);
		pge::Vec3f rotE = rotC.getEulerAngles();

		obs[0] = pos.getX();
		obs[1] = pos.getZ();
		obs[2] = vel.getX();
		obs[3] = vel.getZ();
		obs[4] = rotE.x;
		obs[5] = rotE.z;
		obs[6] = angleVel.getX();
		obs[7] = angleVel.getZ();

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
}

void SceneObjectCartPole::deferredRender() {
	pge::SceneObjectStaticModelBatcher* pBatcher = static_cast<pge::SceneObjectStaticModelBatcher*>(_batcherRef.get());

	{
		// Render cart
		pge::Matrix4x4f transform;

		_pRigidBodyCart->getWorldTransform().getOpenGLMatrix(&transform._elements[0]);

		_pCartModel->render(pBatcher, transform);
	}

	{
		// Render pole
		pge::Matrix4x4f transform;

		_pRigidBodyPole->getWorldTransform().getOpenGLMatrix(&transform._elements[0]);

		transform *= pge::Matrix4x4f::translateMatrix(pge::Vec3f(0.0f, -0.5f, 0.0f));

		_pPoleModel->render(pBatcher, transform);
	}
}

void SceneObjectCartPole::postRender() {
	// Get data from effect buffer
	glReadBuffer(GL_FRONT);

	glReadPixels(0, 0, getRenderScene()->_gBuffer.getWidth(), getRenderScene()->_gBuffer.getHeight(), GL_RGB, GL_UNSIGNED_BYTE, _capBytes->data());
}

void SceneObjectCartPole::onDestroy() {
	if (_socket != nullptr)
		_socket->disconnect();

	if (_physicsWorld.isAlive()) {
		pge::SceneObjectPhysicsWorld* pPhysicsWorld = static_cast<pge::SceneObjectPhysicsWorld*>(_physicsWorld.get());

		if (_pConstraint != nullptr)
			pPhysicsWorld->_pDynamicsWorld->removeConstraint(_pConstraint.get());

		if (_pRigidBodyFloor != nullptr)
			pPhysicsWorld->_pDynamicsWorld->removeRigidBody(_pRigidBodyFloor.get());

		if (_pRigidBodyCart != nullptr)
			pPhysicsWorld->_pDynamicsWorld->removeRigidBody(_pRigidBodyCart.get());

		if (_pRigidBodyPole != nullptr)
			pPhysicsWorld->_pDynamicsWorld->removeRigidBody(_pRigidBodyPole.get());
	}
}
