#pragma once

#include <SFML/Network.hpp>

#include <pge/scene/RenderScene.h>

#include <pge/sceneobjects/SceneObjectProp.h>

class SceneObjectCartPole : public pge::SceneObject {
private:
	// Rendering
	pge::StaticModelOBJ* _pCartModel;
	pge::StaticModelOBJ* _pPoleModel;

	pge::SceneObjectRef _batcherRef;

	pge::Vec2f _action;

	int _ticksPerAction;
	int _ticks;

	pge::Vec2f _cartAccel;
	pge::Vec2f _cartPos;
	pge::Vec2f _cartVel;
	pge::Vec2f _poleAngle;
	pge::Vec2f _poleVel;

	float _reward;

	bool _show;

	bool _doneLastFrame;

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

	SceneObjectCartPole() {
		_renderMask = 0xffff;
	}

	bool create();

	// Inherited from SceneObject
	void onAdd();
	void synchronousUpdate(float dt);
	void deferredRender();
	void onDestroy();
	void postRender();

	void reset();

	SceneObjectCartPole* copyFactory() {
		return new SceneObjectCartPole(*this);
	}
};

