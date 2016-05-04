#include <pge/sceneobjects/SceneObjectBlockGame.h>

#include <pge/rendering/model/SceneObjectStaticModelBatcher.h>

#include <iostream>
#include <sstream>

bool SceneObjectBlockGame::create(int size, int numStartBlocks) {
	assert(getScene() != nullptr);

	_size = size;
	_numStartBlocks = numStartBlocks;

	// Rendering
	std::shared_ptr<pge::Asset> asset;

	if (!getScene()->getAssetManager("MOBJ", pge::StaticModelOBJ::assetFactory)->getAsset("resources/models/block.obj", asset))
		return false;

	_pBlockModel = static_cast<pge::StaticModelOBJ*>(asset.get());

	if (!getScene()->getAssetManager("MOBJ", pge::StaticModelOBJ::assetFactory)->getAsset("resources/models/bot.obj", asset))
		return false;

	_pAgentModel = static_cast<pge::StaticModelOBJ*>(asset.get());

	reset();

	_capture = false;

	_capBytes = std::make_shared<std::vector<char>>(getRenderScene()->_gBuffer.getWidth() * getRenderScene()->_gBuffer.getHeight() * 3, 0);

	_show = getRenderScene()->_renderingEnabled;

	_socket = std::make_shared<sf::TcpSocket>();
    printf("_socket->connect()\n");
	_socket->connect(sf::IpAddress::LocalHost, _port);
    printf("_socket->connect() connected\n");

	return true;
}

void SceneObjectBlockGame::onAdd() {
	_batcherRef = getScene()->getNamed("smb");

	assert(_batcherRef.isAlive());
}

void SceneObjectBlockGame::reset() {
	// Generate a random map
	_rng.seed(_gameSeed);

	_blocks.clear();
	_blocks.assign(_size * _size, 0);

	std::uniform_int_distribution<int> blockDist(0, _blocks.size() - 1);

	for (int i = 0; i < _numStartBlocks; i++) {
		int randSpot = blockDist(_rng);

		_blocks[randSpot]++;
	}

	_agentPosition = blockDist(_rng);

	_action = 0;
	_ticksPerAction = 5;
	_ticks = 0;
	_reward = 0.0f;
}

void SceneObjectBlockGame::act() {
	int agentX = _agentPosition % _size;
	int agentY = _agentPosition / _size;

	int oldPosX = agentX;
	int oldPosY = agentY;

	switch (_action) {
	// Moves
	case 0:
		agentY++;

		break;
	case 1:
		agentY--;

		break;
	case 2:
		agentX++;

		break;
	case 3:
		agentX--;

		break;
	// Pushes
	case 4:
		// If is valid push direction
		if (agentY + 2 < _size) {
			int &pushHeight = _blocks[(agentX) + (agentY + 1) * _size];
			int &targetHeight = _blocks[(agentX) + (agentY + 2) * _size];

			// If a block exists
			if (pushHeight > 0) {
				// If on same level as agent
				if (_blocks[_agentPosition] - 1 == pushHeight) {
					// If not too high
					if (targetHeight <= pushHeight + 1) {
						// Push over
						pushHeight--;
						targetHeight++;
					}
				}
			}
		}

		break;

	case 5:
		// If is valid push direction
		if (agentY - 2 >= 0) {
			int &pushHeight = _blocks[(agentX)+(agentY - 1) * _size];
			int &targetHeight = _blocks[(agentX)+(agentY - 2) * _size];

			// If a block exists
			if (pushHeight > 0) {
				// If on same level as agent
				if (_blocks[_agentPosition] - 1 == pushHeight) {
					// If not too high
					if (targetHeight <= pushHeight + 1) {
						// Push over
						pushHeight--;
						targetHeight++;
					}
				}
			}
		}

		break;

	case 6:
		// If is valid push direction
		if (agentX + 2 < _size) {
			int &pushHeight = _blocks[(agentX + 1)+(agentY) * _size];
			int &targetHeight = _blocks[(agentX + 2)+(agentY) * _size];

			// If a block exists
			if (pushHeight > 0) {
				// If on same level as agent
				if (_blocks[_agentPosition] - 1 == pushHeight) {
					// If not too high
					if (targetHeight <= pushHeight + 1) {
						// Push over
						pushHeight--;
						targetHeight++;
					}
				}
			}
		}

		break;

	case 7:
		// If is valid push direction
		if (agentX - 2 >= 0) {
			int &pushHeight = _blocks[(agentX - 1) + (agentY)* _size];
			int &targetHeight = _blocks[(agentX - 2) + (agentY)* _size];

			// If a block exists
			if (pushHeight > 0) {
				// If on same level as agent
				if (_blocks[_agentPosition] - 1 == pushHeight) {
					// If not too high
					if (targetHeight <= pushHeight + 1) {
						// Push over
						pushHeight--;
						targetHeight++;
					}
				}
			}
		}

		break;
	}

	// See if new position is valid
	if (agentX < 0 || agentX >= _size || agentY < 0 || agentY >= _size) {
		agentX = oldPosX;
		agentY = oldPosY;
	}

	// If moved, but new position is too high
	if (agentX != oldPosX || agentY != oldPosY) {
		if (_blocks[agentX + agentY * _size] > _blocks[oldPosX + oldPosY * _size] + 1) {
			// Reset position
			agentX = oldPosX;
			agentY = oldPosY;
		}
	}

	// Write out new position
	_agentPosition = agentX + agentY * _size;

	_reward = _blocks[_agentPosition];
}

void SceneObjectBlockGame::synchronousUpdate(float dt) {
	if (_ticks >= _ticksPerAction || !getRenderScene()->_renderingEnabled) {
		_ticks = 0;

		std::array<char, _maxBatchSize> buffer;

		// Receive 1 byte
		size_t received = 0;

		_socket->receive(buffer.data(), 1 + 4, received);
		
		if (buffer[0] == 'A') { // Action
			_action = *reinterpret_cast<int*>(&buffer[1]);	
		}
		else if (buffer[0] == 'R') { // Reset
			reset();
		}
		else if (buffer[0] == 'C') { // Capture + action
			_action = *reinterpret_cast<int*>(&buffer[1]);

			_capture = true;

			getRenderScene()->_renderingEnabled = true;
		}
		else if (buffer[0] == 'S') { // Stop capture + action
			_action = *reinterpret_cast<int*>(&buffer[1]);

			_capture = false;

			if (!_show) {
				getRenderScene()->_renderingEnabled = false;
			}
		}
		else if (buffer[0] == 'X') { // Exit
			getRenderScene()->_close = true;
		}

		act();

		// Give state and reward (+ capture if is on)

		// Observation (values of 9 fields around agent)
		std::vector<int> obs(9);

		int obsIndex = 0;

		int agentX = _agentPosition % _size;
		int agentY = _agentPosition / _size;

		for (int dx = -1; dx <= 1; dx++)
			for (int dy = -1; dy <= 1; dy++) {
				int tx = agentX + dx;
				int ty = agentY + dy;

				if (tx >= 0 && ty >= 0 && tx < _size && ty < _size) {
					obs[obsIndex++] = _blocks[tx + ty * _size];
				}
				else
					obs[obsIndex++] = -1; // -1 for wall
			}

		// First add reward
		int index = 0;

		*reinterpret_cast<float*>(&buffer[index]) = _reward;

		index += sizeof(float);

		for (int i = 0; i < obs.size(); i++) {
			*reinterpret_cast<int*>(&buffer[index]) = obs[i];

			index += sizeof(int);
		}

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
				for (int x = 0; x < getRenderScene()->_gBuffer.getWidth(); x++)	{
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

void SceneObjectBlockGame::deferredRender() {
	pge::SceneObjectStaticModelBatcher* pBatcher = static_cast<pge::SceneObjectStaticModelBatcher*>(_batcherRef.get());

	// Render blocks
	for (int i = 0; i < _blocks.size(); i++) {
		int x = i % _size;
		int y = i / _size;

		float xf = x - _size * 0.5f;
		float yf = y - _size * 0.5f;

		for (int j = 0; j < _blocks[i]; j++) {
			pge::Matrix4x4f transform = pge::Matrix4x4f::translateMatrix(pge::Vec3f(xf, j + 0.9f, yf));

			_pBlockModel->render(pBatcher, transform);
		}
	}

	// Render agent
	{
		int x = _agentPosition % _size;
		int y = _agentPosition / _size;

		float xf = x - _size * 0.5f;
		float yf = y - _size * 0.5f;

		pge::Matrix4x4f transform = pge::Matrix4x4f::translateMatrix(pge::Vec3f(xf, _blocks[_agentPosition] + 0.9f, yf));

		_pAgentModel->render(pBatcher, transform);
	}
}

void SceneObjectBlockGame::postRender() {
	// Get data from effect buffer
	glReadBuffer(GL_FRONT);

	glReadPixels(0, 0, getRenderScene()->_gBuffer.getWidth(), getRenderScene()->_gBuffer.getHeight(), GL_RGB, GL_UNSIGNED_BYTE, _capBytes->data());
}

void SceneObjectBlockGame::onDestroy() {
	if (_socket != nullptr)
		_socket->disconnect();
}