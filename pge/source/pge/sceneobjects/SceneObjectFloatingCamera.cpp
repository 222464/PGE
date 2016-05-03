#include <pge/sceneobjects/SceneObjectFloatingCamera.h>

#include <pge/scene/RenderScene.h>

#include <pge/util/Math.h>

#include <pge/rendering/lighting/SceneObjectPointLight.h>

SceneObjectFloatingCamera::SceneObjectFloatingCamera()
: _sensitivity(0.01f), _angleX(0.0f), _angleY(0.0f), _velocity(0.0f, 0.0f, 0.0f),
_acceleration(120.0f), _deceleration(10.0f), _runMultiplier(8.0f),
_acceptingInput(false)
{}

void SceneObjectFloatingCamera::onAdd() {
	_input = getScene()->getNamed("buffIn");
}

void SceneObjectFloatingCamera::update(float dt) {
	pge::SceneObjectBufferedInput* pBufferedInput = static_cast<pge::SceneObjectBufferedInput*>(_input.get());

	if (pBufferedInput->isKeyPressed(sf::Keyboard::Escape)) {
		_acceptingInput = !_acceptingInput;

		getRenderScene()->getRenderWindow()->setMouseCursorVisible(!_acceptingInput);
	}

	if (_acceptingInput) {
		sf::Vector2i mousePosition = sf::Mouse::getPosition(*getRenderScene()->getRenderWindow()) - sf::Vector2i(128, 128);
		sf::Mouse::setPosition(sf::Vector2i(128, 128), *getRenderScene()->getRenderWindow());

		_angleX -= mousePosition.x * _sensitivity;
		_angleY -= mousePosition.y * _sensitivity;

		_angleX = std::fmodf(_angleX, pge::_piTimes2);

		if (_angleY < -pge::_piOver2)
			_angleY = -pge::_piOver2;
		else if (_angleY > pge::_piOver2)
			_angleY = pge::_piOver2;

		getRenderScene()->_logicCamera._rotation = pge::Quaternion(_angleX, pge::Vec3f(0.0f, 1.0f, 0.0f)) * pge::Quaternion(_angleY, pge::Vec3f(1.0f, 0.0f, 0.0f));

		float accel = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ? _acceleration * _runMultiplier : _acceleration;

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
			_velocity += getRenderScene()->_logicCamera._rotation * pge::Vec3f(0.0f, 0.0f, -accel * dt);
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
			_velocity += getRenderScene()->_logicCamera._rotation * pge::Vec3f(0.0f, 0.0f, accel * dt);

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
			_velocity += getRenderScene()->_logicCamera._rotation * pge::Vec3f(accel * dt, 0.0f, 0.0f);
		else if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
			_velocity += getRenderScene()->_logicCamera._rotation * pge::Vec3f(-accel * dt, 0.0f, 0.0f);
	}

	_velocity += -_deceleration * _velocity * dt;

	getRenderScene()->_logicCamera._position += _velocity * dt;
}