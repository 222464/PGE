#pragma once

#include <SFML/Network.hpp>

#include "../scene/RenderScene.h"

#include "SceneObjectProp.h"

class SceneObjectSurf : public pge::SceneObject {
private:
	// Rendering
	pge::StaticModelOBJ* pBlockModel;
	pge::StaticModelOBJ* pAgentModel;

	pge::SceneObjectRef batcherRef;

	int action;

	int ticksPerAction;
	int ticks;

	int size;
	int numStartBlocks;
	int agentPosition;

	float reward;

	bool show;

	std::vector<int> blocks;

	std::mt19937 rng;

	bool capture;
	std::shared_ptr<std::vector<char>> capBytes;

	std::shared_ptr<sf::TcpSocket> socket;

	void act();

public:
	static const unsigned short port = 54003;

	static const unsigned int maxBatchSize = 16384;

	static const unsigned int gameSeed = 1234;

	SceneObjectSurf() {
		renderMask = 0xffff;
	}

	bool create();

	// Inherited from SceneObject
	void onAdd();
	void synchronousUpdate(float dt);
	void deferredRender();
	void onDestroy();
	void postRender();

	void reset();

	SceneObjectSurf* copyFactory() {
		return new SceneObjectSurf(*this);
	}
};

