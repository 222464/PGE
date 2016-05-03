#pragma once

#include <pge/scene/Scene.h>

class SceneObjectGUIRenderable : public pge::SceneObject {
private:
	pge::SceneObjectRef _gui;

public:
	virtual void guiOnAdd() {};
	virtual void guiRender(sf::RenderTexture &renderTexture) {};

	// Inherited from SceneObject
	void onAdd();
	void deferredRender();

	const pge::SceneObjectRef &getGUI() const {
		return _gui;
	}
};