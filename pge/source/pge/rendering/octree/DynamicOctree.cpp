#include <pge/rendering/octree/DynamicOctree.h>

#include <pge/scene/SceneObjectRef.h>
#include <pge/scene/SceneObject.h>

#include <pge/util/Math.h>

#include <assert.h>

using namespace pge;

void DynamicOctree::operator=(const DynamicOctree &other) {
	Octree::operator=(other);

	_minOutsideRoot = other._minOutsideRoot;
	_maxOutsideRoot = other._maxOutsideRoot;
}

void DynamicOctree::add(const SceneObjectRef &oc) {
	std::unique_lock<std::recursive_mutex> lock(_mutex);

	assert(created());

	// If the occupant fits in the root node
	if(_pRootNode->getRegion().contains(oc->getAABB()))
		_pRootNode->add(oc);
	else
		_outsideRoot.insert(oc);

	setOctree(oc);
}

void DynamicOctree::expand() {
	std::unique_lock<std::recursive_mutex> lock(_mutex);

	// Find direction with most occupants
	Vec3f averageDir(0.0f, 0.0f, 0.0f);

	for (std::unordered_set<SceneObjectRef, SceneObjectRef>::iterator it = _outsideRoot.begin(); it != _outsideRoot.end(); it++)
		averageDir += ((*it)->getAABB().getCenter() - _pRootNode->getRegion().getCenter()).normalized();

	Vec3f centerOffsetDist(_pRootNode->getRegion().getHalfDims() / _oversizeMultiplier);

	Vec3f centerOffset(sign(averageDir.x) * centerOffsetDist.x, sign(averageDir.y) * centerOffsetDist.y, sign(averageDir.z) * centerOffsetDist.z);

	// Child node position of current root node
	int rX = centerOffset.x > 0.0f ? 0 : 1;
	int rY = centerOffset.y > 0.0f ? 0 : 1;
	int rZ = centerOffset.z > 0.0f ? 0 : 1;

	AABB3D newRootAABB(Vec3f(0.0f, 0.0f, 0.0f), centerOffsetDist * 4.0f);
	newRootAABB.setCenter(centerOffset + _pRootNode->getRegion().getCenter());

	OctreeNode* pNewRoot = new OctreeNode(newRootAABB,  _pRootNode->_level + 1, nullptr, this);

	// ----------------------- Manual Children Creation for New Root -------------------------

	const Vec3f &halfRegionDims(pNewRoot->_region.getHalfDims());
	const Vec3f &regionLowerBound(pNewRoot->_region.getLowerBound());
	const Vec3f &regionCenter(pNewRoot->_region.getCenter());

	// Create the children nodes
	for(int x = 0; x < 2; x++)
		for(int y = 0; y < 2; y++)
			for(int z = 0; z < 2; z++) {
				if(x == rX && y == rY && z == rZ)
					pNewRoot->_children[x + y * 2 + z * 4].reset(_pRootNode.release());
				else {
					Vec3f offset(x * halfRegionDims.x, y * halfRegionDims.y, z * halfRegionDims.z);

					AABB3D childAABB(regionLowerBound + offset, regionCenter + offset);

					// Scale up AABB by the oversize multiplier
					childAABB.setHalfDims(childAABB.getHalfDims() * _oversizeMultiplier);
					childAABB.calculateCenter();
	
					pNewRoot->_children[x + y * 2 + z * 4].reset(new OctreeNode(childAABB, _pRootNode->_level, pNewRoot, this));
				}
			}

	pNewRoot->_hasChildren = true;
	pNewRoot->_numOccupantsBelow = _pRootNode->_numOccupantsBelow;
	_pRootNode->_pParent = pNewRoot;

	// Transfer ownership
	_pRootNode.release();
	_pRootNode.reset(pNewRoot);

	// ----------------------- Try to Add Previously Outside Root -------------------------

	// Make copy so don't try to re-add ones just added
	std::unordered_set<SceneObjectRef, SceneObjectRef> outsideRootCopy(_outsideRoot);
	_outsideRoot.clear();

	for (std::unordered_set<SceneObjectRef, SceneObjectRef>::iterator it = outsideRootCopy.begin(); it != outsideRootCopy.end(); it++)
		add(*it);
}

void DynamicOctree::contract() {
	std::unique_lock<std::recursive_mutex> lock(_mutex);

	assert(_pRootNode->_hasChildren);

	// Find child with the most occupants and shrink to that
	int maxIndex = 0;

	for (int i = 1; i < 8; i++)
	if (_pRootNode->_children[i]->getNumOccupantsBelow() >
		_pRootNode->_children[maxIndex]->getNumOccupantsBelow())
		maxIndex = i;

	// Reorganize
	for (int i = 0; i < 8; i++) {
		if (i == maxIndex)
			continue;

		_pRootNode->_children[i]->removeForDeletion(_outsideRoot);
	}

	OctreeNode* pNewRoot = _pRootNode->_children[maxIndex].release();

	_pRootNode->destroyChildren();

	_pRootNode->removeForDeletion(_outsideRoot);

	_pRootNode.reset(pNewRoot);

	_pRootNode->_pParent = nullptr;
}

void DynamicOctree::trim() {
	std::unique_lock<std::recursive_mutex> lock(_mutex);

	if(_pRootNode.get() == nullptr)
		return;

	// Check if should grow
	if(_outsideRoot.size() > _maxOutsideRoot)
		expand();
	else if(_outsideRoot.size() < _minOutsideRoot && _pRootNode->_hasChildren)
		contract();
}