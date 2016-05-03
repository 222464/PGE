#pragma once

#include <pge/scene/Scene.h>

#include <pge/constructs/Matrix4x4f.h>

#include <pge/rendering/model/StaticModelOBJ.h>

#include <iostream>

class SceneObjectProp : public pge::SceneObject {
private:
	pge::StaticModelOBJ* _pModelOBJ;

	pge::SceneObjectRef _batcherRef;

public:
	pge::Matrix4x4f _transform;

	SceneObjectProp() {
		_renderMask = 0xffff;
	}

	bool create(const std::string &fileName);

	void calculateAABB();

	// Inherited from SceneObject
	void onAdd();
	void deferredRender();

	pge::StaticModelOBJ* getModel() {
		return _pModelOBJ;
	}

	SceneObject* copyFactory() {
		return new SceneObjectProp(*this);
	}
};