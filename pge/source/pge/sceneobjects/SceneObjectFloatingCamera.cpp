#include "SceneObjectFloatingCamera.h"

#include "../scene/RenderScene.h"

#include "../util/Math.h"

#include "../rendering/lighting/SceneObjectPointLight.h"

SceneObjectFloatingCamera::SceneObjectFloatingCamera()
: sensitivity(0.01f), angleX(0.0f), angleY(0.0f), velocity(0.0f, 0.0f, 0.0f),
acceleration(120.0f), deceleration(10.0f), runMultiplier(8.0f),
acceptingInput(false)
{}

void SceneObjectFloatingCamera::onAdd() {
    input = getScene()->getNamed("buffIn");
}

void SceneObjectFloatingCamera::update(float dt) {
    pge::SceneObjectBufferedInput* pBufferedInput = static_cast<pge::SceneObjectBufferedInput*>(input.get());

    if (pBufferedInput->isKeyPressed(sf::Keyboard::Escape)) {
        acceptingInput = !acceptingInput;

        getRenderScene()->getRenderWindow()->setMouseCursorVisible(!acceptingInput);
    }

    if (acceptingInput) {
        sf::Vector2i mousePosition = sf::Mouse::getPosition(*getRenderScene()->getRenderWindow()) - sf::Vector2i(128, 128);
        sf::Mouse::setPosition(sf::Vector2i(128, 128), *getRenderScene()->getRenderWindow());

        angleX -= mousePosition.x * sensitivity;
        angleY -= mousePosition.y * sensitivity;

        angleX = fmodf(angleX, pge::piTimes2);

        if (angleY < -pge::piOver2)
            angleY = -pge::piOver2;
        else if (angleY > pge::piOver2)
            angleY = pge::piOver2;

        getRenderScene()->logicCamera.rotation = pge::Quaternion(angleX, pge::Vec3f(0.0f, 1.0f, 0.0f)) * pge::Quaternion(angleY, pge::Vec3f(1.0f, 0.0f, 0.0f));

        float accel = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ? acceleration * runMultiplier : acceleration;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
            velocity += getRenderScene()->logicCamera.rotation * pge::Vec3f(0.0f, 0.0f, -accel * dt);
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
            velocity += getRenderScene()->logicCamera.rotation * pge::Vec3f(0.0f, 0.0f, accel * dt);

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
            velocity += getRenderScene()->logicCamera.rotation * pge::Vec3f(accel * dt, 0.0f, 0.0f);
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
            velocity += getRenderScene()->logicCamera.rotation * pge::Vec3f(-accel * dt, 0.0f, 0.0f);
    }

    velocity += -deceleration * velocity * dt;

    getRenderScene()->logicCamera.position += velocity * dt;
}