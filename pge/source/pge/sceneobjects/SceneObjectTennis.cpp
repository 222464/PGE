#include "SceneObjectTennis.h"

#include "../rendering/model/SceneObjectStaticModelBatcher.h"

#include "../util/Math.h"

#include <iostream>
#include <sstream>

bool SceneObjectTennis::create() {
	assert(getScene() != nullptr);

	// Rendering
	std::shared_ptr<pge::Asset> asset;

	if (!getScene()->getAssetManager("MOBJ", pge::StaticModelOBJ::assetFactory)->getAsset("resources/models/slime3D.obj", asset))
		return false;

	pSlimeModel = static_cast<pge::StaticModelOBJ*>(asset.get());

	if (!getScene()->getAssetManager("MOBJ", pge::StaticModelOBJ::assetFactory)->getAsset("resources/models/tennisBall.obj", asset))
		return false;

	pBallModel = static_cast<pge::StaticModelOBJ*>(asset.get());

	// Get reference to physics world
	physicsWorld = getScene()->getNamedCheckQueue("physWrld");

	reset();

	capture = false;

	capBytes = std::make_shared<std::vector<char>>(getRenderScene()->gBuffer.getWidth() * getRenderScene()->gBuffer.getHeight() * 3, 0);

	show = getRenderScene()->renderingEnabled;

	socket = std::make_shared<sf::TcpSocket>();

	socket->connect(sf::IpAddress::LocalHost, port, sf::seconds(5.0f));

	doneLastFrame = false;

	return true;
}

void SceneObjectTennis::onAdd() {
	batcherRef = getScene()->getNamed("smb");

	assert(batcherRef.isAlive());
}

void SceneObjectTennis::reset() {
	// Slightly random angle
	std::uniform_real_distribution<float> pertDist(-0.05f, 0.05f);

	assert(physicsWorld.isAlive());

	pge::SceneObjectPhysicsWorld* pPhysicsWorld = static_cast<pge::SceneObjectPhysicsWorld*>(physicsWorld.get());

	// Remove old
	if (pRigidBodySlime != nullptr)
		pPhysicsWorld->pDynamicsWorld->removeRigidBody(pRigidBodySlime.get());

	if (pRigidBodyBall != nullptr)
		pPhysicsWorld->pDynamicsWorld->removeRigidBody(pRigidBodyBall.get());

	// Physics
	pCollisionShapeSlime.reset(new btSphereShape(0.5f));
	pCollisionShapeBall.reset(new btSphereShape(0.1f));

	btVector3 slimeStart = btVector3(-3.0f, 0.0f, 0.0f);

	std::uniform_real_distribution<float> heightDist(1.2f, 3.0f);
	std::uniform_real_distribution<float> widthDist(-2.0f, 2.0f);
	std::uniform_real_distribution<float> velPertDist(-2.5f, 2.5f);
	btVector3 startVel = btVector3(-5.0f + velPertDist(rng), velPertDist(rng), velPertDist(rng));
	btVector3 ballStart = btVector3(0.0f, heightDist(rng), widthDist(rng));

	pMotionStateSlime.reset(new btDefaultMotionState(btTransform(btQuaternion(0.0f, 0.0f, 0.0f), slimeStart)));
	pMotionStateBall.reset(new btDefaultMotionState(btTransform(btQuaternion(0.0f, 0.0f, 0.0f), ballStart)));

	const float slimeMass = 10.0f;
	const float ballMass = 1.0f;

	btVector3 slimeInertia, ballInertia;

	pCollisionShapeSlime->calculateLocalInertia(slimeMass, slimeInertia);
	pCollisionShapeBall->calculateLocalInertia(ballMass, ballInertia);

	btRigidBody::btRigidBodyConstructionInfo rigidBodyCISlime(slimeMass, pMotionStateSlime.get(), pCollisionShapeSlime.get(), slimeInertia);
	btRigidBody::btRigidBodyConstructionInfo rigidBodyCIBall(ballMass, pMotionStateBall.get(), pCollisionShapeBall.get(), ballInertia);

	rigidBodyCISlime.m_restitution = 2.0f;
	rigidBodyCISlime.m_friction = 0.01f;

	rigidBodyCIBall.m_restitution = 1.0f;
	rigidBodyCIBall.m_friction = 0.01f;

	pRigidBodySlime.reset(new btRigidBody(rigidBodyCISlime));
	pRigidBodyBall.reset(new btRigidBody(rigidBodyCIBall));

	pRigidBodyBall->setLinearVelocity(startVel);

	pPhysicsWorld->pDynamicsWorld->addRigidBody(pRigidBodySlime.get());
	pPhysicsWorld->pDynamicsWorld->addRigidBody(pRigidBodyBall.get());

	pRigidBodySlime->setAngularFactor(0.0f); // No rotation
	pRigidBodySlime->setLinearFactor(btVector3(1.0f, 0.0f, 1.0f)); // No movement up and down
	pRigidBodySlime->setDamping(0.07f, 0.0f);

	action = pge::Vec2f(0.0f, 0.0f);
	ticksPerAction = 0;
	ticks = 0;
	reward = 0.0f;
}

void SceneObjectTennis::act() {
	const float force = 500.0f;
	const float maxSpeed = 5.0f;

	float speed = pRigidBodySlime->getLinearVelocity().length();

	pRigidBodySlime->applyCentralForce(btVector3(action.x, 0.0f, action.y) * force * (maxSpeed - speed) / maxSpeed);

	btVector3 slimePos = pRigidBodySlime->getWorldTransform().getOrigin();
	btVector3 ballPos = pRigidBodyBall->getWorldTransform().getOrigin();

	reward = 0.0f;

	if (ballPos.getX() > 0.0f && pRigidBodyBall->getLinearVelocity().getX() > 0.0f) {
		reset();

		reward = 1.0f;

		doneLastFrame = true;
	}
	else if (ballPos.getY() < 0.0f) {
		reset();

		reward = -1.0f;

		doneLastFrame = true;
	}
}

void SceneObjectTennis::synchronousUpdate(float dt) {
	if (ticks >= ticksPerAction || !getRenderScene()->renderingEnabled) {
		ticks = 0;

		std::array<char, maxBatchSize> buffer;

		std::array<char, 1 + 4 + 4> msg;

		size_t received = 0;
		size_t totalReceived = 0;

		while (totalReceived < msg.size()) {
			socket->receive(buffer.data(), msg.size() - totalReceived, received);

			for (int i = 0; i < received; i++)
				msg[totalReceived + i] = buffer[i];

			totalReceived += received;
		}

		if (msg[0] == 'A') { // Action
			action = pge::Vec2f(*reinterpret_cast<float*>(&msg[1]), *reinterpret_cast<float*>(&msg[5]));
		}
		else if (msg[0] == 'R') { // Reset
			action = pge::Vec2f(*reinterpret_cast<float*>(&msg[1]), *reinterpret_cast<float*>(&msg[5]));

			reset();
		}
		else if (msg[0] == 'C') { // Capture + action
			action = pge::Vec2f(*reinterpret_cast<float*>(&msg[1]), *reinterpret_cast<float*>(&msg[5]));

			capture = true;

			if (!getRenderScene()->renderingEnabled) {
				getRenderScene()->getRenderWindow()->setFramerateLimit(60);
				getRenderScene()->getRenderWindow()->setVerticalSyncEnabled(true);
			}

			getRenderScene()->renderingEnabled = true;
		}
		else if (msg[0] == 'S') { // Stop capture + action
			action = pge::Vec2f(*reinterpret_cast<float*>(&msg[1]), *reinterpret_cast<float*>(&msg[5]));

			capture = false;

			if (!show) {
				if (getRenderScene()->renderingEnabled) {
					getRenderScene()->getRenderWindow()->setFramerateLimit(0);
					getRenderScene()->getRenderWindow()->setVerticalSyncEnabled(false);
				}

				getRenderScene()->renderingEnabled = false;
			}
		}
		else if (msg[0] == 'X') { // Exit
			getRenderScene()->close = true;
		}

		action.x = std::min(1.0f, std::max(-1.0f, action.x));
		action.y = std::min(1.0f, std::max(-1.0f, action.y));

		act();

		// Give state and reward (+ capture if is on)

		// Observation (8 values)
		std::vector<float> obs(10);

		btVector3 slimePos = pRigidBodySlime->getWorldTransform().getOrigin();
		btVector3 slimeVel = pRigidBodySlime->getLinearVelocity();
		btVector3 toBall = pRigidBodyBall->getWorldTransform().getOrigin() - slimePos;
		btVector3 ballVel = pRigidBodyBall->getLinearVelocity();

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

		*reinterpret_cast<float*>(&buffer[index]) = reward;

		index += sizeof(float);

		for (int i = 0; i < obs.size(); i++) {
			*reinterpret_cast<float*>(&buffer[index]) = obs[i];

			index += sizeof(float);
		}

		// Reset flag
		*reinterpret_cast<int*>(&buffer[index]) = static_cast<int>(doneLastFrame);

		doneLastFrame = false;

		index += sizeof(int);

		// Submit number of batches of maxBatchSize
		int numBatches = capBytes->size() / maxBatchSize + ((capBytes->size() % maxBatchSize) == 0 ? 0 : 1);

		// No batches if not capturing
		if (!capture)
			numBatches = 0;

		*reinterpret_cast<int*>(&buffer[index]) = numBatches;

		index += sizeof(int);

		socket->send(buffer.data(), index);

		if (capture) {
			std::vector<char> reorganized(capBytes->size());

			int reorgIndex = 0;

			for (int y = 0; y < getRenderScene()->gBuffer.getHeight(); y++)
				for (int x = 0; x < getRenderScene()->gBuffer.getWidth(); x++) {
					int start = 3 * (x + (getRenderScene()->gBuffer.getHeight() - 1 - y) * getRenderScene()->gBuffer.getWidth());

					reorganized[reorgIndex++] = (*capBytes)[start + 0];
					reorganized[reorgIndex++] = (*capBytes)[start + 1];
					reorganized[reorgIndex++] = (*capBytes)[start + 2];
				}

			int total = 0;

			for (int i = 0; i < numBatches; i++) {
				// Submit batch
				size_t count = 0;

				for (int j = 0; j < maxBatchSize && total < capBytes->size(); j++) {
					buffer[j] = reorganized[total++];

					count++;
				}

				socket->send(buffer.data(), count);
			}
		}
	}
	else
		ticks++;
}

void SceneObjectTennis::deferredRender() {
	pge::SceneObjectStaticModelBatcher* pBatcher = static_cast<pge::SceneObjectStaticModelBatcher*>(batcherRef.get());

	{
		// Render slime
		pge::Matrix4x4f transform;

		pRigidBodySlime->getWorldTransform().getOpenGLMatrix(&transform.elements[0]);

		float s = 0.5f * 2.0f;

		transform *= pge::Matrix4x4f::scaleMatrix(pge::Vec3f(s, s, s));

		pSlimeModel->render(pBatcher, transform);
	}

	{
		// Render ball
		pge::Matrix4x4f transform;

		pRigidBodyBall->getWorldTransform().getOpenGLMatrix(&transform.elements[0]);

		float s = 0.1f * 2.0f;

		transform *= pge::Matrix4x4f::scaleMatrix(pge::Vec3f(s, s, s));

		pBallModel->render(pBatcher, transform);
	}
}

void SceneObjectTennis::postRender() {
	// Get data from effect buffer
	glReadPixels(0, 0, getRenderScene()->gBuffer.getWidth(), getRenderScene()->gBuffer.getHeight(), GL_RGB, GL_UNSIGNED_BYTE, capBytes->data());
}

void SceneObjectTennis::onDestroy() {
	if (socket != nullptr)
		socket->disconnect();

	if (physicsWorld.isAlive()) {
		pge::SceneObjectPhysicsWorld* pPhysicsWorld = static_cast<pge::SceneObjectPhysicsWorld*>(physicsWorld.get());

		if (pRigidBodySlime != nullptr)
			pPhysicsWorld->pDynamicsWorld->removeRigidBody(pRigidBodySlime.get());

		if (pRigidBodyBall != nullptr)
			pPhysicsWorld->pDynamicsWorld->removeRigidBody(pRigidBodyBall.get());
	}
}
