#pragma once

#include <pge/scene/Scene.h>

namespace pge {
	class SceneObjectBufferedInput : public SceneObject {
	private:
		std::vector<bool> _prevKeyStates;
		std::vector<bool> _currentKeyStates;

		bool _prevLMBDown;
		bool _prevRMBDown;
		bool _prevMMBDown;

		bool _currentLMBDown;
		bool _currentRMBDown;
		bool _currentMMBDown;

		int _mouseWheelDelta;

	public:
		std::vector<sf::Event> _events;

		SceneObjectBufferedInput();

		// Inherited from SceneObject
		void synchronousUpdate(float dt);

		bool isKeyDown(sf::Keyboard::Key key) const {
			return _currentKeyStates[key];
		}

		bool wasKeyDown(sf::Keyboard::Key key) const {
			return _prevKeyStates[key];
		}

		bool isKeyPressed(sf::Keyboard::Key key) const {
			return _currentKeyStates[key] && !_prevKeyStates[key];
		}

		bool isKeyReleased(sf::Keyboard::Key key) const {
			return !_currentKeyStates[key] && _prevKeyStates[key];
		}

		bool isLMBDown() const {
			return _currentLMBDown;
		}

		bool wasLMBDown() const {
			return _prevLMBDown;
		}

		bool isLMBPressed() const {
			return _currentLMBDown && !_prevLMBDown;
		}

		bool isLMBReleased() const {
			return !_currentLMBDown && _prevLMBDown;
		}

		bool isRMBDown() const {
			return _currentRMBDown;
		}

		bool wasRMBDown() const {
			return _prevRMBDown;
		}

		bool isRMBPressed() const {
			return _currentRMBDown && !_prevRMBDown;
		}

		bool isRMBReleased() const {
			return !_currentRMBDown && _prevRMBDown;
		}

		bool isMMBDown() const {
			return _currentMMBDown;
		}

		bool wasMMBDown() const {
			return _prevMMBDown;
		}

		bool isMMBPressed() const {
			return _currentMMBDown && !_prevMMBDown;
		}

		bool isMMBReleased() const {
			return !_currentMMBDown && _prevMMBDown;
		}

		int getMouseWheelDelta() const {
			return _mouseWheelDelta;
		}

		SceneObject* copyFactory() {
			return new SceneObjectBufferedInput(*this);
		}
	};
}