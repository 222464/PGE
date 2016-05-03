#include <pge/sceneobjects/input/SceneObjectBufferedInput.h>

using namespace pge;

SceneObjectBufferedInput::SceneObjectBufferedInput() {
	_prevKeyStates.assign(sf::Keyboard::KeyCount, false);
	_currentKeyStates.assign(sf::Keyboard::KeyCount, false);

	_prevLMBDown = false;
	_prevRMBDown = false;
	_prevMMBDown = false;

	_currentLMBDown = false;
	_currentRMBDown = false;
	_currentMMBDown = false;

	_layer = 1.0f;
}

void SceneObjectBufferedInput::synchronousUpdate(float dt) {
	for (size_t i = 0; i < _currentKeyStates.size(); i++) {
		_prevKeyStates[i] = _currentKeyStates[i];

		_currentKeyStates[i] = sf::Keyboard::isKeyPressed(static_cast<sf::Keyboard::Key>(i));
	}

	_prevLMBDown = _currentLMBDown;
	_prevRMBDown = _currentRMBDown;
	_prevMMBDown = _currentMMBDown;

	_currentLMBDown = sf::Mouse::isButtonPressed(sf::Mouse::Left);
	_currentRMBDown = sf::Mouse::isButtonPressed(sf::Mouse::Right);
	_currentMMBDown = sf::Mouse::isButtonPressed(sf::Mouse::Middle);

	_mouseWheelDelta = 0;

	for (size_t i = 0; i < _events.size(); i++)
	if (_events[i].type == sf::Event::MouseWheelMoved)
		_mouseWheelDelta += _events[i].mouseWheel.delta;

	_events.clear();
}