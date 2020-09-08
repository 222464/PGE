#include "StaticOctree.h"

#include "../../scene/SceneObject.h"

#include <assert.h>

using namespace pge;

void StaticOctree::add(const SceneObjectRef &oc) {
	std::unique_lock<std::recursive_mutex> lock(mutex);

	assert(created());

	setOctree(oc);

	// If the occupant fits in the root node
	if(pRootNode->getRegion().contains(oc->getAABB()))
		pRootNode->add(oc);
	else
		outsideRoot.insert(oc);
}