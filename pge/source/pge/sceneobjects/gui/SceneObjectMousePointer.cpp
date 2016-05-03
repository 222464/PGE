#include <pge/sceneobjects/gui/SceneObjectMousePointer.h>

#include <pge/scene/RenderScene.h>

void SceneObjectMousePointer::create(const std::shared_ptr<pge::Texture2D> &pointerTexture) {
	_pointerTexture = pointerTexture;

	_layer = 1.0f;
}

void SceneObjectMousePointer::guiOnAdd() {
	getRenderScene()->getRenderWindow()->setMouseCursorVisible(false);
}

void SceneObjectMousePointer::guiRender(sf::RenderTexture &renderTexture) {
	sf::Sprite sprite;

	sprite.setTexture(_pointerTexture->getTexture());

	sf::Vector2i mousePos = sf::Mouse::getPosition(*getRenderScene()->getRenderWindow());

	sprite.setPosition(sf::Vector2f(mousePos.x, mousePos.y));
	sprite.setScale(sf::Vector2f(3.0f, 3.0f));

	renderTexture.draw(sprite);
}

void SceneObjectMousePointer::synchronousUpdate(float dt) {

}