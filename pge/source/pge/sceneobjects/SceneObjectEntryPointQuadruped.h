#pragma once

#include <pge/scene/Scene.h>

class SceneObjectEntryPointQuadruped : public pge::SceneObject {
public:
	// Inherited from SceneObject
	void onAdd();

	SceneObject* copyFactory() {
		return new SceneObjectEntryPointQuadruped(*this);
	}
};