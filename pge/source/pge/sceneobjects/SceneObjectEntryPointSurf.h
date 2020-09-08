#pragma once

#include "../scene/Scene.h"

class SceneObjectEntryPointSurf : public pge::SceneObject {
public:
	// Inherited from SceneObject
	void onAdd();

	SceneObject* copyFactory() {
		return new SceneObjectEntryPointSurf(*this);
	}
};