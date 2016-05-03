#pragma once

#include <pge/sceneobjects/gui/SceneObjectGUIRenderable.h>

#include <pge/rendering/texture/Texture2D.h>

class SceneObjectMousePointer : public SceneObjectGUIRenderable {
private:
	std::shared_ptr<pge::Texture2D> _pointerTexture;

public:
	SceneObjectMousePointer() {
		_renderMask = 0xffff;
	}

	void create(const std::shared_ptr<pge::Texture2D> &pointerTexture);

	// Inherited from SceneObjectGUIRenderable
	void guiOnAdd();
	void guiRender(sf::RenderTexture &renderTexture);

	void synchronousUpdate(float dt);

	SceneObjectMousePointer* copyFactory() {
		return new SceneObjectMousePointer(*this);
	}
};