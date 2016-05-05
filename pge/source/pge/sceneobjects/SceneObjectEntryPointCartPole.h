#pragma once

#include <pge/scene/Scene.h>

class SceneObjectEntryPointCartPole : public pge::SceneObject {
public:
	// Inherited from SceneObject
	void onAdd();

	SceneObject* copyFactory() {
		return new SceneObjectEntryPointCartPole(*this);
	}
};