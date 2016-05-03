#pragma once

#include <SFML/Network.hpp>

#include <pge/scene/RenderScene.h>

#include <pge/sceneobjects/SceneObjectProp.h>

class SceneObjectBlockGame : public pge::SceneObject {
private:
	// Rendering
	pge::StaticModelOBJ* _pBlockModel;
	pge::StaticModelOBJ* _pAgentModel;

	pge::SceneObjectRef _batcherRef;

	int _action;

	int _ticksPerAction;
	int _ticks;

	int _size;
	int _numStartBlocks;
	int _agentPosition;

	float _reward;

	bool _show;

	std::vector<int> _blocks;

	std::mt19937 _rng;

	bool _capture;
	std::shared_ptr<std::vector<char>> _capBytes;

	std::shared_ptr<sf::TcpSocket> _socket;

	void act();

public:
	static const unsigned short _port = 54003;

	static const unsigned int _maxBatchSize = 16384;

	static const unsigned int _gameSeed = 1234;

	SceneObjectBlockGame() {
		_renderMask = 0xffff;
	}

	bool create(int size, int numStartBlocks);

	// Inherited from SceneObject
	void onAdd();
	void synchronousUpdate(float dt);
	void deferredRender();
	void onDestroy();
	void postRender();

	void reset();

	SceneObjectBlockGame* copyFactory() {
		return new SceneObjectBlockGame(*this);
	}
};

