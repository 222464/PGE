#pragma once

#include <pge/scene/Scene.h>

class SceneObjectEntryPointBlockGame : public pge::SceneObject {
public:
	// Inherited from SceneObject
	void onAdd();

	SceneObject* copyFactory() {
		return new SceneObjectEntryPointBlockGame(*this);
	}
};