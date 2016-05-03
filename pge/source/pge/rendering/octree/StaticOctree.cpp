#include <pge/rendering/octree/StaticOctree.h>

#include <pge/scene/SceneObject.h>

#include <assert.h>

using namespace pge;

void StaticOctree::add(const SceneObjectRef &oc) {
	std::unique_lock<std::recursive_mutex> lock(_mutex);

	assert(created());

	setOctree(oc);

	// If the occupant fits in the root node
	if(_pRootNode->getRegion().contains(oc->getAABB()))
		_pRootNode->add(oc);
	else
		_outsideRoot.insert(oc);
}