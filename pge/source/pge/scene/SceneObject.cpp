#include <pge/scene/SceneObject.h>

#include <pge/scene/RenderScene.h>

using namespace pge;

SceneObject::SceneObject()
: _aabb(Vec3f(0.0f, 0.0f, 0.0f), Vec3f(1.0f, 1.0f, 1.0f)),
_pOctreeNode(nullptr), _pOctree(nullptr),
_pScene(nullptr), _indexPlusOne(0), _layer(0.0f),
_logicMask(0xffff), _renderMask(0x0000),
_tag(""), _needsTreeUpdate(false),
_syncable(false), _shouldDestroy(false),
_mutex(new std::recursive_mutex())
{}

RenderScene* SceneObject::getRenderScene() const {
	return static_cast<RenderScene*>(_pScene);
}

void SceneObject::treeUpdate() {
	if (_pOctree == nullptr)
		return;

	std::unique_lock<std::recursive_mutex> lock(_pOctree->_mutex);

	if (_pOctreeNode == nullptr) {
		// Not in a node, should be outside root then

		// If fits in the root now, add it
		OctreeNode* pRootNode = _pOctree->_pRootNode.get();

		if (pRootNode->_region.contains(_aabb))
			pRootNode->add(SceneObjectRef(this));
	}
	else
		_pOctreeNode->update(SceneObjectRef(this));
}

void SceneObject::removeReferences() {
	std::unique_lock<std::recursive_mutex> lock(*_mutex);

	// Nullify all references to this. Must copy since the references list will be modified when removing
	std::unordered_set<SceneObjectRef*> pReferencesCopy = _pReferences;

	for (SceneObjectRef* pRef : pReferencesCopy)
		*pRef = nullptr;

	_pReferences.clear();
}