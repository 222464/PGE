#pragma once

#include <pge/scene/RenderScene.h>

#include <pge/sceneobjects/input/SceneObjectBufferedInput.h>

class SceneObjectGUI : public pge::SceneObject {
private:
	pge::SceneObjectRef _bufferedInput;

	std::list<pge::SceneObjectRef> _guiRenderables;

	std::shared_ptr<sf::RenderTexture> _renderTexture;

	std::shared_ptr<pge::Shader> _renderImageShader;

	GLuint _renderTextureID;

public:
	bool _enabled;

	SceneObjectGUI();

	// Inherited from SceneObject
	void onAdd();
	void update(float dt);
	void postRender();

	sf::RenderTexture* getRenderTexture() const {
		return _renderTexture.get();
	}

	SceneObjectGUI* copyFactory() {
		return new SceneObjectGUI(*this);
	}

	friend class SceneObjectGUIRenderable;
};

