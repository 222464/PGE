#pragma once

#include <pge/scene/Scene.h>

#include <pge/constructs/Matrix4x4f.h>

#include <pge/rendering/model/StaticModelOBJ.h>

#include <iostream>

class SceneObjectPropLOD : public pge::SceneObject {
private:
	std::vector<pge::StaticModelOBJ*> _pModelsOBJ;

	pge::SceneObjectRef _batcherRef;

public:
	pge::Matrix4x4f _transform;

	float _lodSwitchDistance;

	SceneObjectPropLOD()
		: _lodSwitchDistance(20.0f)
	{
		_renderMask = 0xffff;
	}

	bool create(const std::vector<std::string> &fileNames);

	void calculateAABB();

	// Inherited from SceneObject
	void onAdd();
	void deferredRender();

	SceneObject* copyFactory() {
		return new SceneObjectPropLOD(*this);
	}
};