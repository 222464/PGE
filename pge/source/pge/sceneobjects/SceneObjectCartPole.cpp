#include <pge/sceneobjects/SceneObjectCartPole.h>

#include <pge/rendering/model/SceneObjectStaticModelBatcher.h>

#include <pge/util/Math.h>

#include <iostream>
#include <sstream>

bool SceneObjectCartPole::create() {
	assert(getScene() != nullptr);

	// Rendering
	std::shared_ptr<pge::Asset> asset;

	if (!getScene()->getAssetManager("MOBJ", pge::StaticModelOBJ::assetFactory)->getAsset("resources/models/cart.obj", asset))
		return false;

	_pCartModel = static_cast<pge::StaticModelOBJ*>(asset.get());

	if (!getScene()->getAssetManager("MOBJ", pge::StaticModelOBJ::assetFactory)->getAsset("resources/models/pole.obj", asset))
		return false;

	_pPoleModel = static_cast<pge::StaticModelOBJ*>(asset.get());

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

	// Generate a random map
	_cartAccel = pge::Vec2f(0.0f, 0.0f);
	_cartPos = pge::Vec2f(0.0f, 0.0f);
	_cartVel = pge::Vec2f(0.0f, 0.0f);
	_poleAngle = pge::Vec2f(pertDist(_rng), pertDist(_rng));
	_poleVel = pge::Vec2f(0.0f, 0.0f);

	_action = pge::Vec2f(0.0f, 0.0f);
	_ticksPerAction = 0;
	_ticks = 0;
	_reward = 0.0f;
}

void SceneObjectCartPole::act() {
	const float dt = 0.017f;
	const float g = 9.81f;
	const float poleRotationalFriction = 0.01f;
	const float cartFriction = 0.01f;
	const float maxSpeed = 3.0f;
	const float agentForce = 3000.0f;
	const float poleLength = 1.0f;
	const float massMass = 10.0f;
	const float cartMass = 10.0f;

	float poleAngleAccelX = _cartAccel.x * std::cos(_poleAngle.x) + g * std::sin(_poleAngle.x);
	float poleAngleAccelY = _cartAccel.y * std::cos(_poleAngle.y) + g * std::sin(_poleAngle.y);
	_poleVel += -poleRotationalFriction * _poleVel + pge::Vec2f(poleAngleAccelX, poleAngleAccelY) * dt;
	_poleAngle += _poleVel * dt;

	float forceX = 0.0f;

	if (std::abs(_action.x) < maxSpeed)
		forceX = std::max(-agentForce, std::min(agentForce, agentForce * _action.x));

	float forceY = 0.0f;

	if (std::abs(_action.y) < maxSpeed)
		forceY = std::max(-agentForce, std::min(agentForce, agentForce * _action.y));

	_cartAccel.x = 0.25f * (forceX + massMass * poleLength * poleAngleAccelX * std::cos(_poleAngle.x) - massMass * poleLength * _poleVel.x * _poleVel.x * std::sin(_poleAngle.x)) / (massMass + cartMass);
	_cartAccel.y = 0.25f * (forceY + massMass * poleLength * poleAngleAccelY * std::cos(_poleAngle.y) - massMass * poleLength * _poleVel.y * _poleVel.y * std::sin(_poleAngle.y)) / (massMass + cartMass);
	_cartVel += -cartFriction * _cartVel + pge::Vec2f(_cartAccel.x, _cartAccel.y) * dt;
	_cartPos += _cartVel * dt;

	_poleAngle.x = std::fmod(_poleAngle.x, (2.0f * pge::_pi));
	_poleAngle.y = std::fmod(_poleAngle.y, (2.0f * pge::_pi));

	if (_poleAngle.x < 0.0f)
		_poleAngle.x += pge::_pi * 2.0f;

	if (_poleAngle.y < 0.0f)
		_poleAngle.y += pge::_pi * 2.0f;

	const float angleTolerance = 0.8f;
	const float positionTolerance = 3.5f;

	float nAngleX = _poleAngle.x > pge::_pi ? pge::_piTimes2 - _poleAngle.x : _poleAngle.x;
	float nAngleY = _poleAngle.y > pge::_pi ? pge::_piTimes2 - _poleAngle.y : _poleAngle.y;

	_reward = -nAngleX - nAngleY;

	if (std::abs(nAngleX) > angleTolerance || std::abs(nAngleY) > angleTolerance ||
		std::abs(_cartPos.x) > positionTolerance || std::abs(_cartPos.y) > positionTolerance) {
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

		obs[0] = _cartPos.x;
		obs[1] = _cartPos.y;
		obs[2] = _cartVel.x;
		obs[3] = _cartVel.y;
		obs[4] = _poleAngle.x;
		obs[5] = _poleAngle.y;
		obs[6] = _poleVel.x;
		obs[7] = _poleVel.y;

		// First add reward
		int index = 0;

		*reinterpret_cast<float*>(&buffer[index]) = _reward;

		index += sizeof(float);

		for (int i = 0; i < obs.size(); i++) {
			*reinterpret_cast<float*>(&buffer[index]) = obs[i];

			index += sizeof(float);
		}

		// Reset flag
		*reinterpret_cast<char*>(&buffer[index]) = static_cast<char>(_doneLastFrame);

		_doneLastFrame = false;

		index += sizeof(char);

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
		pge::Matrix4x4f transform = pge::Matrix4x4f::translateMatrix(pge::Vec3f(_cartPos.x, 0.9f, _cartPos.y));

		_pCartModel->render(pBatcher, transform);
	}

	{
		// Render pole
		pge::Matrix4x4f transform = pge::Matrix4x4f::translateMatrix(pge::Vec3f(_cartPos.x, 1.0f, _cartPos.y)) * pge::Matrix4x4f::rotateMatrixX(_poleAngle.x) * pge::Matrix4x4f::rotateMatrixZ(_poleAngle.y);

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
}