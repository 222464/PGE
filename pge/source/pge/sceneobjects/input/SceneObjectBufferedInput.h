#pragma once

#include "../../scene/Scene.h"

namespace pge {
    class SceneObjectBufferedInput : public SceneObject {
    private:
        std::vector<bool> prevKeyStates;
        std::vector<bool> currentKeyStates;

        bool prevLMBDown;
        bool prevRMBDown;
        bool prevMMBDown;

        bool currentLMBDown;
        bool currentRMBDown;
        bool currentMMBDown;

        int mouseWheelDelta;

    public:
        std::vector<sf::Event> events;

        SceneObjectBufferedInput();

        // Inherited from SceneObject
        void synchronousUpdate(float dt);

        bool isKeyDown(sf::Keyboard::Key key) const {
            return currentKeyStates[key];
        }

        bool wasKeyDown(sf::Keyboard::Key key) const {
            return prevKeyStates[key];
        }

        bool isKeyPressed(sf::Keyboard::Key key) const {
            return currentKeyStates[key] && !prevKeyStates[key];
        }

        bool isKeyReleased(sf::Keyboard::Key key) const {
            return !currentKeyStates[key] && prevKeyStates[key];
        }

        bool isLMBDown() const {
            return currentLMBDown;
        }

        bool wasLMBDown() const {
            return prevLMBDown;
        }

        bool isLMBPressed() const {
            return currentLMBDown && !prevLMBDown;
        }

        bool isLMBReleased() const {
            return !currentLMBDown && prevLMBDown;
        }

        bool isRMBDown() const {
            return currentRMBDown;
        }

        bool wasRMBDown() const {
            return prevRMBDown;
        }

        bool isRMBPressed() const {
            return currentRMBDown && !prevRMBDown;
        }

        bool isRMBReleased() const {
            return !currentRMBDown && prevRMBDown;
        }

        bool isMMBDown() const {
            return currentMMBDown;
        }

        bool wasMMBDown() const {
            return prevMMBDown;
        }

        bool isMMBPressed() const {
            return currentMMBDown && !prevMMBDown;
        }

        bool isMMBReleased() const {
            return !currentMMBDown && prevMMBDown;
        }

        int getMouseWheelDelta() const {
            return mouseWheelDelta;
        }

        SceneObject* copyFactory() {
            return new SceneObjectBufferedInput(*this);
        }
    };
}