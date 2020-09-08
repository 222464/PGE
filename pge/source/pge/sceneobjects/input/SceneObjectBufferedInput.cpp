#include "SceneObjectBufferedInput.h"

using namespace pge;

SceneObjectBufferedInput::SceneObjectBufferedInput() {
	prevKeyStates.assign(sf::Keyboard::KeyCount, false);
	currentKeyStates.assign(sf::Keyboard::KeyCount, false);

	prevLMBDown = false;
	prevRMBDown = false;
	prevMMBDown = false;

	currentLMBDown = false;
	currentRMBDown = false;
	currentMMBDown = false;

	layer = 1.0f;
}

void SceneObjectBufferedInput::synchronousUpdate(float dt) {
	for (size_t i = 0; i < currentKeyStates.size(); i++) {
		prevKeyStates[i] = currentKeyStates[i];

		currentKeyStates[i] = sf::Keyboard::isKeyPressed(static_cast<sf::Keyboard::Key>(i));
	}

	prevLMBDown = currentLMBDown;
	prevRMBDown = currentRMBDown;
	prevMMBDown = currentMMBDown;

	currentLMBDown = sf::Mouse::isButtonPressed(sf::Mouse::Left);
	currentRMBDown = sf::Mouse::isButtonPressed(sf::Mouse::Right);
	currentMMBDown = sf::Mouse::isButtonPressed(sf::Mouse::Middle);

	mouseWheelDelta = 0;

	for (size_t i = 0; i < events.size(); i++)
	if (events[i].type == sf::Event::MouseWheelMoved)
		mouseWheelDelta += events[i].mouseWheel.delta;

	events.clear();
}