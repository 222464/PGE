#include <pge/sceneobjects/SceneObjectTennis.h>

#include <pge/rendering/model/SceneObjectStaticModelBatcher.h>

#include <pge/util/Math.h>

#include <iostream>
#include <sstream>

bool SceneObjectTennis::create() {
	assert(getScene() != nullptr);

	// Rendering
	std::shared_ptr<pge::Asset> asset;

	if (!getScene()->getAssetManager("MOBJ", pge::StaticModelOBJ::assetFactory)->getAsset("resources/models/slime3D.obj", asset))
		return false;

	_pSlimeModel = static_cast<pge::StaticModelOBJ*>(asset.get());

	if (!getScene()->getAssetManager("MOBJ", pge::StaticModelOBJ::assetFactory)->getAsset("resources/models/tennisBall.obj", asset))
		return false;

	_pBallModel = static_cast<pge::StaticModelOBJ*>(asset.get());

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

void SceneObjectTennis::onAdd() {
	_batcherRef = getScene()->getNamed("smb");

	assert(_batcherRef.isAlive());
}

void SceneObjectTennis::reset() {
	// Slightly random angle
	std::uniform_real_distribution<float> pertDist(-0.05f, 0.05f);

	assert(_physicsWorld.isAlive());

	pge::SceneObjectPhysicsWorld* pPhysicsWorld = static_cast<pge::SceneObjectPhysicsWorld*>(_physicsWorld.get());

	// Remove old
	if (_pRigidBodySlime != nullptr)
		pPhysicsWorld->_pDynamicsWorld->removeRigidBody(_pRigidBodySlime.get());

	if (_pRigidBodyBall != nullptr)
		pPhysicsWorld->_pDynamicsWorld->removeRigidBody(_pRigidBodyBall.get());

	// Physics
	_pCollisionShapeSlime.reset(new btSphereShape(0.5f));
	_pCollisionShapeBall.reset(new btSphereShape(0.1f));

	btVector3 slimeStart = btVector3(-3.0f, 0.0f, 0.0f);

	std::uniform_real_distribution<float> heightDist(1.2f, 3.0f);
	std::uniform_real_distribution<float> widthDist(-2.0f, 2.0f);
	std::uniform_real_distribution<float> velPertDist(-2.5f, 2.5f);
	btVector3 startVel = btVector3(-5.0f + velPertDist(_rng), velPertDist(_rng), velPertDist(_rng));
	btVector3 ballStart = btVector3(0.0f, heightDist(_rng), widthDist(_rng));

	_pMotionStateSlime.reset(new btDefaultMotionState(btTransform(btQuaternion(0.0f, 0.0f, 0.0f), slimeStart)));
	_pMotionStateBall.reset(new btDefaultMotionState(btTransform(btQuaternion(0.0f, 0.0f, 0.0f), ballStart)));

	const float slimeMass = 10.0f;
	const float ballMass = 1.0f;

	btVector3 slimeInertia, ballInertia;

	_pCollisionShapeSlime->calculateLocalInertia(slimeMass, slimeInertia);
	_pCollisionShapeBall->calculateLocalInertia(ballMass, ballInertia);

	btRigidBody::btRigidBodyConstructionInfo rigidBodyCISlime(slimeMass, _pMotionStateSlime.get(), _pCollisionShapeSlime.get(), slimeInertia);
	btRigidBody::btRigidBodyConstructionInfo rigidBodyCIBall(ballMass, _pMotionStateBall.get(), _pCollisionShapeBall.get(), ballInertia);

	rigidBodyCISlime.m_restitution = 2.0f;
	rigidBodyCISlime.m_friction = 0.01f;

	rigidBodyCIBall.m_restitution = 1.0f;
	rigidBodyCIBall.m_friction = 0.01f;

	_pRigidBodySlime.reset(new btRigidBody(rigidBodyCISlime));
	_pRigidBodyBall.reset(new btRigidBody(rigidBodyCIBall));

	_pRigidBodyBall->setLinearVelocity(startVel);

	pPhysicsWorld->_pDynamicsWorld->addRigidBody(_pRigidBodySlime.get());
	pPhysicsWorld->_pDynamicsWorld->addRigidBody(_pRigidBodyBall.get());

	_pRigidBodySlime->setAngularFactor(0.0f); // No rotation
	_pRigidBodySlime->setLinearFactor(btVector3(1.0f, 0.0f, 1.0f)); // No movement up and down
	_pRigidBodySlime->setDamping(0.07f, 0.0f);

	_action = pge::Vec2f(0.0f, 0.0f);
	_ticksPerAction = 0;
	_ticks = 0;
	_reward = 0.0f;
}

void SceneObjectTennis::act() {
	const float force = 500.0f;
	const float maxSpeed = 5.0f;

	float speed = _pRigidBodySlime->getLinearVelocity().length();

	_pRigidBodySlime->applyCentralForce(btVector3(_action.x, 0.0f, _action.y) * force * (maxSpeed - speed) / maxSpeed);

	btVector3 slimePos = _pRigidBodySlime->getWorldTransform().getOrigin();
	btVector3 ballPos = _pRigidBodyBall->getWorldTransform().getOrigin();

	_reward = 0.0f;

	if (ballPos.getX() > 0.0f && _pRigidBodyBall->getLinearVelocity().getX() > 0.0f) {
		reset();

		_reward = 1.0f;

		_doneLastFrame = true;
	}
	else if (ballPos.getY() < 0.0f) {
		reset();

		_reward = -1.0f;

		_doneLastFrame = true;
	}
}

void SceneObjectTennis::synchronousUpdate(float dt) {
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

			if (!getRenderScene()->_renderingEnabled) {
				getRenderScene()->getRenderWindow()->setFramerateLimit(60);
				getRenderScene()->getRenderWindow()->setVerticalSyncEnabled(true);
			}

			getRenderScene()->_renderingEnabled = true;
		}
		else if (msg[0] == 'S') { // Stop capture + action
			_action = pge::Vec2f(*reinterpret_cast<float*>(&msg[1]), *reinterpret_cast<float*>(&msg[5]));

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

		_action.x = std::min(1.0f, std::max(-1.0f, _action.x));
		_action.y = std::min(1.0f, std::max(-1.0f, _action.y));

		act();

		// Give state and reward (+ capture if is on)

		// Observation (8 values)
		std::vector<float> obs(10);

		btVector3 slimePos = _pRigidBodySlime->getWorldTransform().getOrigin();
		btVector3 slimeVel = _pRigidBodySlime->getLinearVelocity();
		btVector3 toBall = _pRigidBodyBall->getWorldTransform().getOrigin() - slimePos;
		btVector3 ballVel = _pRigidBodyBall->getLinearVelocity();

		obs[0] = slimePos.getX();
		obs[1] = slimePos.getZ();
		obs[2] = slimeVel.getX();
		obs[3] = slimeVel.getZ();
		obs[4] = toBall.getX();
		obs[5] = toBall.getY();
		obs[6] = toBall.getZ();
		obs[7] = ballVel.getX();
		obs[8] = ballVel.getY();
		obs[9] = ballVel.getZ();

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

void SceneObjectTennis::deferredRender() {
	pge::SceneObjectStaticModelBatcher* pBatcher = static_cast<pge::SceneObjectStaticModelBatcher*>(_batcherRef.get());

	{
		// Render slime
		pge::Matrix4x4f transform;

		_pRigidBodySlime->getWorldTransform().getOpenGLMatrix(&transform._elements[0]);

		float s = 0.5f * 2.0f;

		transform *= pge::Matrix4x4f::scaleMatrix(pge::Vec3f(s, s, s));

		_pSlimeModel->render(pBatcher, transform);
	}

	{
		// Render ball
		pge::Matrix4x4f transform;

		_pRigidBodyBall->getWorldTransform().getOpenGLMatrix(&transform._elements[0]);

		float s = 0.1f * 2.0f;

		transform *= pge::Matrix4x4f::scaleMatrix(pge::Vec3f(s, s, s));

		_pBallModel->render(pBatcher, transform);
	}
}

void SceneObjectTennis::postRender() {
	// Get data from effect buffer
	glReadBuffer(GL_FRONT);

	glReadPixels(0, 0, getRenderScene()->_gBuffer.getWidth(), getRenderScene()->_gBuffer.getHeight(), GL_RGB, GL_UNSIGNED_BYTE, _capBytes->data());
}

void SceneObjectTennis::onDestroy() {
	if (_socket != nullptr)
		_socket->disconnect();

	if (_physicsWorld.isAlive()) {
		pge::SceneObjectPhysicsWorld* pPhysicsWorld = static_cast<pge::SceneObjectPhysicsWorld*>(_physicsWorld.get());

		if (_pRigidBodySlime != nullptr)
			pPhysicsWorld->_pDynamicsWorld->removeRigidBody(_pRigidBodySlime.get());

		if (_pRigidBodyBall != nullptr)
			pPhysicsWorld->_pDynamicsWorld->removeRigidBody(_pRigidBodyBall.get());
	}
}
