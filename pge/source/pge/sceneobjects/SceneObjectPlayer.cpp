#include "SceneObjectPlayer.h"

#include "../scene/RenderScene.h"

#include "../util/Math.h"

SceneObjectPlayer::SceneObjectPlayer() 
: sensitivity(0.01f), angleX(0.0f), angleY(0.0f), noClipVelocity(0.0f, 0.0f, 0.0f),
noClipAcceleration(120.0f), noClipDeceleration(10.0f), noClipRunMultiplier(8.0f),
acceleration(20.0f), deceleration(4.0f), runMultiplier(2.5f),
radius(0.5f), height(2.0f), mass(70.0f), stepHeight(0.2f),
cameraHeightOffset(1.0f),
allowNoclipChange(true), acceptingInput(false),
lastMousePosition(128, 128)
{}

void SceneObjectPlayer::setNoClip(bool noClip) {
    if (noClip)
        characterController.reset();
    else {
        assert(getScene() != nullptr);

        pge::SceneObjectRef physicsWorld = getScene()->getNamed("physWrld");

        assert(physicsWorld.isAlive());

        characterController.reset(new pge::DynamicCharacterController(getScene(), static_cast<pge::SceneObjectPhysicsWorld*>(physicsWorld.get()), getScene()->logicCamera.position + pge::Vec3f(0.0f, -cameraHeightOffset, 0.0f), radius, height, mass, stepHeight));
    }
}

void SceneObjectPlayer::onAdd() {
    input = getScene()->getNamed("buffIn");

    assert(input.isAlive());
}

void SceneObjectPlayer::update(float dt) {
    pge::SceneObjectBufferedInput* pBufferedInput = static_cast<pge::SceneObjectBufferedInput*>(input.get());

    if (pBufferedInput->isKeyPressed(sf::Keyboard::Escape)) {
        acceptingInput = !acceptingInput;
        
        if (acceptingInput) {
            lastMousePosition = sf::Mouse::getPosition(*getRenderScene()->getRenderWindow());

            sf::Mouse::setPosition(sf::Vector2i(128, 128), *getRenderScene()->getRenderWindow());
        }
        else
            sf::Mouse::setPosition(lastMousePosition, *getRenderScene()->getRenderWindow());
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

        if (allowNoclipChange)
        if (pBufferedInput->isKeyPressed(sf::Keyboard::N))
            setNoClip(characterController != nullptr);

        if (characterController == nullptr) {
            float accel = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ? noClipAcceleration * noClipRunMultiplier : noClipAcceleration;

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
                noClipVelocity += getRenderScene()->logicCamera.rotation * pge::Vec3f(0.0f, 0.0f, -accel * dt);
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
                noClipVelocity += getRenderScene()->logicCamera.rotation * pge::Vec3f(0.0f, 0.0f, accel * dt);

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
                noClipVelocity += getRenderScene()->logicCamera.rotation * pge::Vec3f(accel * dt, 0.0f, 0.0f);
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
                noClipVelocity += getRenderScene()->logicCamera.rotation * pge::Vec3f(-accel * dt, 0.0f, 0.0f);

            noClipVelocity += -noClipDeceleration * noClipVelocity * dt;

            getRenderScene()->logicCamera.position += noClipVelocity * dt;
        }
        else {
            float accel = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ? acceleration * runMultiplier : acceleration;

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
                characterController->walk(getRenderScene()->logicCamera.rotation * pge::Vec3f(0.0f, 0.0f, -accel * dt));
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
                characterController->walk(getRenderScene()->logicCamera.rotation * pge::Vec3f(0.0f, 0.0f, accel * dt));

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
                characterController->walk(getRenderScene()->logicCamera.rotation * pge::Vec3f(accel * dt, 0.0f, 0.0f));
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
                characterController->walk(getRenderScene()->logicCamera.rotation * pge::Vec3f(-accel * dt, 0.0f, 0.0f));

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
                characterController->jump();

            characterController->deceleration = deceleration;

            characterController->update(dt);

            getRenderScene()->logicCamera.position = characterController->getPosition() + pge::Vec3f(0.0f, cameraHeightOffset, 0.0f);
        }
    }
}

void SceneObjectPlayer::synchronousUpdate(float dt) {
    getRenderScene()->getRenderWindow()->setMouseCursorVisible(!acceptingInput);
}