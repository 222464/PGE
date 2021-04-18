#include "SceneObjectBlockGame.h"

#include "../rendering/model/SceneObjectStaticModelBatcher.h"

#include <iostream>
#include <sstream>

bool SceneObjectBlockGame::create(int size, int numStartBlocks) {
    assert(getScene() != nullptr);

    this->size = size;
    this->numStartBlocks = numStartBlocks;

    // Rendering
    std::shared_ptr<pge::Asset> asset;

    if (!getScene()->getAssetManager("MOBJ", pge::StaticModelOBJ::assetFactory)->getAsset("resources/models/block.obj", asset))
        return false;

    pBlockModel = static_cast<pge::StaticModelOBJ*>(asset.get());

    if (!getScene()->getAssetManager("MOBJ", pge::StaticModelOBJ::assetFactory)->getAsset("resources/models/bot.obj", asset))
        return false;

    pAgentModel = static_cast<pge::StaticModelOBJ*>(asset.get());

    reset();

    capture = false;

    capBytes = std::make_shared<std::vector<char>>(getRenderScene()->gBuffer.getWidth() * getRenderScene()->gBuffer.getHeight() * 3, 0);

    show = getRenderScene()->renderingEnabled;

    socket = std::make_shared<sf::TcpSocket>();

    socket->connect(sf::IpAddress::LocalHost, port, sf::seconds(5.0f));

    return true;
}

void SceneObjectBlockGame::onAdd() {
    batcherRef = getScene()->getNamed("smb");

    assert(batcherRef.isAlive());
}

void SceneObjectBlockGame::reset() {
    // Generate a random map
    rng.seed(gameSeed);

    blocks.clear();
    blocks.assign(size * size, 0);

    std::uniform_int_distribution<int> blockDist(0, blocks.size() - 1);

    for (int i = 0; i < numStartBlocks; i++) {
        int randSpot = blockDist(rng);

        blocks[randSpot]++;
    }

    agentPosition = blockDist(rng);

    action = 0;
    ticksPerAction = 5;
    ticks = 0;
    reward = 0.0f;
}

void SceneObjectBlockGame::act() {
    int agentX = agentPosition % size;
    int agentY = agentPosition / size;

    int oldPosX = agentX;
    int oldPosY = agentY;

    switch (action) {
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
        if (agentY + 2 < size) {
            int &pushHeight = blocks[(agentX) + (agentY + 1) * size];
            int &targetHeight = blocks[(agentX) + (agentY + 2) * size];

            // If a block exists
            if (pushHeight > 0) {
                // If on same level as agent
                if (blocks[agentPosition] - 1 == pushHeight) {
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
            int &pushHeight = blocks[(agentX)+(agentY - 1) * size];
            int &targetHeight = blocks[(agentX)+(agentY - 2) * size];

            // If a block exists
            if (pushHeight > 0) {
                // If on same level as agent
                if (blocks[agentPosition] - 1 == pushHeight) {
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
        if (agentX + 2 < size) {
            int &pushHeight = blocks[(agentX + 1)+(agentY) * size];
            int &targetHeight = blocks[(agentX + 2)+(agentY) * size];

            // If a block exists
            if (pushHeight > 0) {
                // If on same level as agent
                if (blocks[agentPosition] - 1 == pushHeight) {
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
            int &pushHeight = blocks[(agentX - 1) + (agentY)* size];
            int &targetHeight = blocks[(agentX - 2) + (agentY)* size];

            // If a block exists
            if (pushHeight > 0) {
                // If on same level as agent
                if (blocks[agentPosition] - 1 == pushHeight) {
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
    if (agentX < 0 || agentX >= size || agentY < 0 || agentY >= size) {
        agentX = oldPosX;
        agentY = oldPosY;
    }

    // If moved, but new position is too high
    if (agentX != oldPosX || agentY != oldPosY) {
        if (blocks[agentX + agentY * size] > blocks[oldPosX + oldPosY * size] + 1) {
            // Reset position
            agentX = oldPosX;
            agentY = oldPosY;
        }
    }

    // Write out new position
    agentPosition = agentX + agentY * size;

    reward = blocks[agentPosition];
}

void SceneObjectBlockGame::synchronousUpdate(float dt) {
    if (ticks >= ticksPerAction || !getRenderScene()->renderingEnabled) {
        ticks = 0;

        std::array<char, maxBatchSize> buffer;

        std::array<char, 1 + 4> msg;

        size_t received = 0;
        size_t totalReceived = 0;

        while (totalReceived < msg.size()) {
            socket->receive(buffer.data(), msg.size() - totalReceived, received);

            for (int i = 0; i < received; i++)
                msg[totalReceived + i] = buffer[i];

            totalReceived += received;
        }

        if (msg[0] == 'A') { // Action
            action = *reinterpret_cast<int*>(&msg[1]);
        }
        else if (msg[0] == 'R') { // Reset
            reset();
        }
        else if (msg[0] == 'C') { // Capture + action
            action = *reinterpret_cast<int*>(&msg[1]);

            capture = true;

            if (!getRenderScene()->renderingEnabled) {
                getRenderScene()->getRenderWindow()->setFramerateLimit(60);
                getRenderScene()->getRenderWindow()->setVerticalSyncEnabled(true);
            }

            getRenderScene()->renderingEnabled = true;
        }
        else if (msg[0] == 'S') { // Stop capture + action
            action = *reinterpret_cast<int*>(&msg[1]);

            capture = false;

            if (!show) {
                if (getRenderScene()->renderingEnabled) {
                    getRenderScene()->getRenderWindow()->setFramerateLimit(0);
                    getRenderScene()->getRenderWindow()->setVerticalSyncEnabled(false);
                }

                getRenderScene()->renderingEnabled = false;
            }
        }
        else if (buffer[0] == 'X') { // Exit
            getRenderScene()->close = true;
        }

        act();

        // Give state and reward (+ capture if is on)

        // Observation (values of 9 fields around agent)
        std::vector<int> obs(9);

        int obsIndex = 0;

        int agentX = agentPosition % size;
        int agentY = agentPosition / size;

        for (int dx = -1; dx <= 1; dx++)
            for (int dy = -1; dy <= 1; dy++) {
                int tx = agentX + dx;
                int ty = agentY + dy;

                if (tx >= 0 && ty >= 0 && tx < size && ty < size) {
                    obs[obsIndex++] = blocks[tx + ty * size];
                }
                else
                    obs[obsIndex++] = -1; // -1 for wall
            }

        // First add reward
        int index = 0;

        *reinterpret_cast<float*>(&buffer[index]) = reward;

        index += sizeof(float);

        for (int i = 0; i < obs.size(); i++) {
            *reinterpret_cast<int*>(&buffer[index]) = obs[i];

            index += sizeof(int);
        }

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
                for (int x = 0; x < getRenderScene()->gBuffer.getWidth(); x++)	{
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

void SceneObjectBlockGame::deferredRender() {
    pge::SceneObjectStaticModelBatcher* pBatcher = static_cast<pge::SceneObjectStaticModelBatcher*>(batcherRef.get());

    // Render blocks
    for (int i = 0; i < blocks.size(); i++) {
        int x = i % size;
        int y = i / size;

        float xf = x - size * 0.5f;
        float yf = y - size * 0.5f;

        for (int j = 0; j < blocks[i]; j++) {
            pge::Matrix4x4f transform = pge::Matrix4x4f::translateMatrix(pge::Vec3f(xf, j + 0.9f, yf));

            pBlockModel->render(pBatcher, transform);
        }
    }

    // Render agent
    {
        int x = agentPosition % size;
        int y = agentPosition / size;

        float xf = x - size * 0.5f;
        float yf = y - size * 0.5f;

        pge::Matrix4x4f transform = pge::Matrix4x4f::translateMatrix(pge::Vec3f(xf, blocks[agentPosition] + 0.9f, yf));

        pAgentModel->render(pBatcher, transform);
    }
}

void SceneObjectBlockGame::postRender() {
    // Get data from effect buffer
    glReadPixels(0, 0, getRenderScene()->gBuffer.getWidth(), getRenderScene()->gBuffer.getHeight(), GL_RGB, GL_UNSIGNED_BYTE, capBytes->data());
}

void SceneObjectBlockGame::onDestroy() {
    if (socket != nullptr)
        socket->disconnect();
}
