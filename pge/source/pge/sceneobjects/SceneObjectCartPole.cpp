#include "SceneObjectCartPole.h"

#include "../rendering/model/SceneObjectStaticModelBatcher.h"

#include "../util/Math.h"

#include <iostream>
#include <sstream>

bool SceneObjectCartPole::create() {
	assert(getScene() != nullptr);

	// Rendering
	std::shared_ptr<pge::Asset> asset;

	if (!getScene()->getAssetManager("MOBJ", pge::StaticModelOBJ::assetFactory)->getAsset("resources/models/Cart.obj", asset))
		return false;

	pCartModel = static_cast<pge::StaticModelOBJ*>(asset.get());

	if (!getScene()->getAssetManager("MOBJ", pge::StaticModelOBJ::assetFactory)->getAsset("resources/models/Pole.obj", asset))
		return false;

	pPoleModel = static_cast<pge::StaticModelOBJ*>(asset.get());

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

void SceneObjectCartPole::onAdd() {
	batcherRef = getScene()->getNamed("smb");

	assert(batcherRef.isAlive());
}

void SceneObjectCartPole::reset() {
	// Slightly random angle
	std::uniform_real_distribution<float> pertDist(-0.05f, 0.05f);

	assert(physicsWorld.isAlive());

	pge::SceneObjectPhysicsWorld* pPhysicsWorld = static_cast<pge::SceneObjectPhysicsWorld*>(physicsWorld.get());

	// Remove old
	if (pConstraint != nullptr)
		pPhysicsWorld->pDynamicsWorld->removeConstraint(pConstraint.get());

	if (pRigidBodyFloor != nullptr)
		pPhysicsWorld->pDynamicsWorld->removeRigidBody(pRigidBodyFloor.get());

	if (pRigidBodyCart != nullptr)
		pPhysicsWorld->pDynamicsWorld->removeRigidBody(pRigidBodyCart.get());

	if (pRigidBodyPole != nullptr)
		pPhysicsWorld->pDynamicsWorld->removeRigidBody(pRigidBodyPole.get());

	// Physics
	pCollisionShapeFloor.reset(new btBoxShape(btVector3(4.0f, 0.5f, 4.0f)));
	pCollisionShapeCart.reset(new btBoxShape(bt(pCartModel->getAABB().getHalfDims())));
	pCollisionShapePole.reset(new btCapsuleShape(0.05f, 1.0f));

	pMotionStateFloor.reset(new btDefaultMotionState(btTransform(btQuaternion(0.0f, 0.0f, 0.0f), btVector3(0.0f, 0.25f, 0.0f))));
	pMotionStateCart.reset(new btDefaultMotionState(btTransform(btQuaternion(0.0f, 0.0f, 0.0f), btVector3(0.0f, 0.85f, 0.0f))));
	pMotionStatePole.reset(new btDefaultMotionState(btTransform(btQuaternion(0.0f, pertDist(rng), pertDist(rng)), btVector3(0.0f, 0.85f + 0.5f + 0.05f, 0.0f))));

	const float floorMass = 0.0f;
	const float cartMass = 10.0f;
	const float poleMass = 10.0f;

	btVector3 floorInertia, cartInertia, poleInertia;

	pCollisionShapeFloor->calculateLocalInertia(floorMass, floorInertia);
	pCollisionShapeCart->calculateLocalInertia(cartMass, cartInertia);
	pCollisionShapePole->calculateLocalInertia(poleMass, poleInertia);

	btRigidBody::btRigidBodyConstructionInfo rigidBodyCIFloor(floorMass, pMotionStateFloor.get(), pCollisionShapeFloor.get(), floorInertia);
	btRigidBody::btRigidBodyConstructionInfo rigidBodyCICart(cartMass, pMotionStateCart.get(), pCollisionShapeCart.get(), cartInertia);
	btRigidBody::btRigidBodyConstructionInfo rigidBodyCIPole(poleMass, pMotionStatePole.get(), pCollisionShapePole.get(), poleInertia);

	rigidBodyCIFloor.m_restitution = 0.001f;
	rigidBodyCIFloor.m_friction = 0.01f;

	rigidBodyCICart.m_restitution = 0.001f;
	rigidBodyCICart.m_friction = 0.01f;

	rigidBodyCIPole.m_restitution = 0.001f;
	rigidBodyCIPole.m_friction = 0.01f;

	pRigidBodyFloor.reset(new btRigidBody(rigidBodyCIFloor));
	pRigidBodyCart.reset(new btRigidBody(rigidBodyCICart));
	pRigidBodyPole.reset(new btRigidBody(rigidBodyCIPole));

	pPhysicsWorld->pDynamicsWorld->addRigidBody(pRigidBodyFloor.get());
	pPhysicsWorld->pDynamicsWorld->addRigidBody(pRigidBodyCart.get());
	pPhysicsWorld->pDynamicsWorld->addRigidBody(pRigidBodyPole.get());

	pRigidBodyCart->setAngularFactor(0.0f); // No rotation

	btTransform frameA = btTransform::getIdentity();
	btTransform frameB = btTransform::getIdentity();

	frameA.setOrigin(btVector3(0.0f, 0.05f, 0.0f));
	frameB.setOrigin(btVector3(0.0f, -0.5f, 0.0f));
	pConstraint.reset(new btConeTwistConstraint(*pRigidBodyCart, *pRigidBodyPole, frameA, frameB));

	pPhysicsWorld->pDynamicsWorld->addConstraint(pConstraint.get(), true);

	action = pge::Vec2f(0.0f, 0.0f);
	ticksPerAction = 0;
	ticks = 0;
	reward = 0.0f;
}

void SceneObjectCartPole::act() {
	const float force = 1000.0f;
	const float positionTolerance = 3.5f;
	const float angleTolerance = 0.7f;

	pRigidBodyCart->applyCentralForce(btVector3(action.x, 0.0f, action.y) * force);

	btVector3 pos = pRigidBodyCart->getWorldTransform().getOrigin();
	btQuaternion rot = pRigidBodyPole->getWorldTransform().getRotation();

	reward = -rot.angleShortestPath(btQuaternion::getIdentity());

	if (reward < -angleTolerance ||
		std::abs(pos.getX()) > positionTolerance || std::abs(pos.getZ()) > positionTolerance) {
		std::cout << "Pole fell or is out of bounds. Resetting..." << std::endl;
		reset();
		doneLastFrame = true;
	}
}

void SceneObjectCartPole::synchronousUpdate(float dt) {
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
		std::vector<float> obs(8);

		btVector3 pos = pRigidBodyCart->getWorldTransform().getOrigin();
		btVector3 vel = pRigidBodyCart->getLinearVelocity();
		btQuaternion rot = pRigidBodyPole->getWorldTransform().getRotation();
		btVector3 angleVel = pRigidBodyPole->getAngularVelocity();

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

void SceneObjectCartPole::deferredRender() {
	pge::SceneObjectStaticModelBatcher* pBatcher = static_cast<pge::SceneObjectStaticModelBatcher*>(batcherRef.get());

	{
		// Render cart
		pge::Matrix4x4f transform;

		pRigidBodyCart->getWorldTransform().getOpenGLMatrix(&transform.elements[0]);

		pCartModel->render(pBatcher, transform);
	}

	{
		// Render pole
		pge::Matrix4x4f transform;

		pRigidBodyPole->getWorldTransform().getOpenGLMatrix(&transform.elements[0]);

		transform *= pge::Matrix4x4f::translateMatrix(pge::Vec3f(0.0f, -0.5f, 0.0f));

		pPoleModel->render(pBatcher, transform);
	}
}

void SceneObjectCartPole::postRender() {
	// Get data from effect buffer
	glReadPixels(0, 0, getRenderScene()->gBuffer.getWidth(), getRenderScene()->gBuffer.getHeight(), GL_RGB, GL_UNSIGNED_BYTE, capBytes->data());
}

void SceneObjectCartPole::onDestroy() {
	if (socket != nullptr)
		socket->disconnect();

	if (physicsWorld.isAlive()) {
		pge::SceneObjectPhysicsWorld* pPhysicsWorld = static_cast<pge::SceneObjectPhysicsWorld*>(physicsWorld.get());

		if (pConstraint != nullptr)
			pPhysicsWorld->pDynamicsWorld->removeConstraint(pConstraint.get());

		if (pRigidBodyFloor != nullptr)
			pPhysicsWorld->pDynamicsWorld->removeRigidBody(pRigidBodyFloor.get());

		if (pRigidBodyCart != nullptr)
			pPhysicsWorld->pDynamicsWorld->removeRigidBody(pRigidBodyCart.get());

		if (pRigidBodyPole != nullptr)
			pPhysicsWorld->pDynamicsWorld->removeRigidBody(pRigidBodyPole.get());
	}
}
