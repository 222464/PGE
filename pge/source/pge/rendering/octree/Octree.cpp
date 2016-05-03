#include <pge/rendering/octree/Octree.h>

#include <pge/scene/SceneObject.h>

#include <algorithm>

#include <assert.h>

using namespace pge;

Octree::Octree()
: _minNumNodeOccupants(3),
_maxNumNodeOccupants(6),
_maxLevels(40),
_oversizeMultiplier(1.0f)
{}

void Octree::operator=(const Octree &other) {
	_minNumNodeOccupants = other._minNumNodeOccupants;
	_maxNumNodeOccupants = other._maxNumNodeOccupants;
	_maxLevels = other._maxLevels;
	_oversizeMultiplier = other._oversizeMultiplier;

	_outsideRoot = other._outsideRoot;

	if (other._pRootNode != nullptr) {
		_pRootNode.reset(new OctreeNode());

		recursiveCopy(_pRootNode.get(), other._pRootNode.get(), nullptr);
	}
}

void Octree::setOctree(const SceneObjectRef &oc) {
	oc->_pOctree = this;
}

void Octree::recursiveCopy(OctreeNode* pThisNode, OctreeNode* pOtherNode, OctreeNode* pThisParent) {
	pThisNode->_hasChildren = pOtherNode->_hasChildren;
	pThisNode->_level = pOtherNode->_level;
	pThisNode->_numOccupantsBelow = pOtherNode->_numOccupantsBelow;
	pThisNode->_occupants = pOtherNode->_occupants;
	pThisNode->_region = pOtherNode->_region;

	pThisNode->_pParent = pThisParent;

	pThisNode->_pOctree = this;

	if (pThisNode->_hasChildren)
	for (int i = 0; i < 8; i++) {
		pThisNode->_children[i].reset(new OctreeNode());

		recursiveCopy(pThisNode->_children[i].get(), pOtherNode->_children[i].get(), pThisNode);
	}
}

void Octree::pruneDeadReferences() {
	for (std::unordered_set<SceneObjectRef, SceneObjectRef>::iterator it = _outsideRoot.begin(); it != _outsideRoot.end();)
	if (it->isAlive())
		it++;
	else
		it = _outsideRoot.erase(it);

	if (_pRootNode != nullptr)
		_pRootNode->pruneDeadReferences();
}

void Octree::queryRegion(const AABB3D &region, std::vector<SceneObjectRef> &result, unsigned short logicMask, unsigned short renderMask) {
	std::lock_guard<std::recursive_mutex> lock(_mutex);

	// Query outside root elements
	for (std::unordered_set<SceneObjectRef, SceneObjectRef>::iterator it = _outsideRoot.begin(); it != _outsideRoot.end(); it++) {
		SceneObjectRef oc = *it;

		if (oc != nullptr && region.intersects(oc->_aabb) && ((renderMask & oc->_renderMask) != 0))
			// Intersects, add to list
			result.push_back(oc);
	}

	std::list<OctreeNode*> open;

	open.push_back(_pRootNode.get());

	while (!open.empty()) {
		// Depth-first (results in less memory usage), remove objects from open list
		OctreeNode* pCurrent = open.back();
		open.pop_back();

		if (region.intersects(pCurrent->_region)) {
			for (std::unordered_set<SceneObjectRef, SceneObjectRef>::iterator it = pCurrent->_occupants.begin(); it != pCurrent->_occupants.end(); it++) {
				SceneObjectRef oc = *it;

				if (oc != nullptr && region.intersects(oc->_aabb) && ((renderMask & oc->_renderMask) != 0))
					// Visible, add to list
					result.push_back(oc);
			}

			// Add children to open list if they intersect the region
			if (pCurrent->_hasChildren)
			for (int i = 0; i < 8; i++)
			if (pCurrent->_children[i]->getNumOccupantsBelow() != 0)
				open.push_back(pCurrent->_children[i].get());
		}
	}
}

void Octree::queryFrustum(std::vector<SceneObjectRef> &result, const Frustum &frustum, unsigned short logicMask, unsigned short renderMask) {
	std::lock_guard<std::recursive_mutex> lock(_mutex);

	// Query outside root elements - add them if they are visible
	for (std::unordered_set<SceneObjectRef, SceneObjectRef>::iterator it = _outsideRoot.begin(); it != _outsideRoot.end(); it++) {
		SceneObjectRef oc = *it;

		if (oc != nullptr && !frustum.testAABBOutside(oc->_aabb) && ((renderMask & oc->_renderMask) != 0))
			// Visible, add to list
			result.push_back(oc);
	}

	std::list<OctreeNode*> open;

	open.push_back(_pRootNode.get());

	while (!open.empty()) {
		// Depth-first (results in less memory usage), remove objects from open list
		OctreeNode* pCurrent = open.back();
		open.pop_back();

		switch (frustum.testAABB(pCurrent->_region)) {
		case Frustum::inside:
			// Add all of this nodes occupants and those below, since all must be visibile
			pCurrent->getAllOccupantsBelow(result);

			break;

		case Frustum::intersect:
			// Add occupants if they are visible
			for (std::unordered_set<SceneObjectRef, SceneObjectRef>::iterator it = pCurrent->_occupants.begin(); it != pCurrent->_occupants.end(); it++) {
				SceneObjectRef oc = *it;

				if (oc != nullptr && !frustum.testAABBOutside(oc->_aabb) && ((renderMask & oc->_renderMask) != 0))
					// Visible, add to list
					result.push_back(oc);
			}

			// Add children to open list
			if (pCurrent->_hasChildren)
			for (int i = 0; i < 8; i++)
			if (pCurrent->_children[i]->getNumOccupantsBelow() != 0)
				open.push_back(pCurrent->_children[i].get());

			break;

			// Outside case is ignored
		}
	}
}

void Octree::querySegment(std::vector<SceneObjectRef> &result, const Vec3f &p1, const Vec3f &p2, unsigned short logicMask, unsigned short renderMask) {
	std::lock_guard<std::recursive_mutex> lock(_mutex);

	// Query outside root elements
	for (std::unordered_set<SceneObjectRef, SceneObjectRef>::iterator it = _outsideRoot.begin(); it != _outsideRoot.end(); it++) {
		SceneObjectRef oc = *it;

		if (oc != nullptr && oc->_aabb.intersects(p1, p2) && ((renderMask & oc->_renderMask) != 0))
			// Intersects, add to list
			result.push_back(oc);
	}

	std::list<OctreeNode*> open;

	open.push_back(_pRootNode.get());

	while (!open.empty()) {
		// Depth-first (results in less memory usage), remove objects from open list
		OctreeNode* pCurrent = open.back();
		open.pop_back();

		if (pCurrent->_region.intersects(p1, p2)) {
			for (std::unordered_set<SceneObjectRef, SceneObjectRef>::iterator it = pCurrent->_occupants.begin(); it != pCurrent->_occupants.end(); it++) {
				SceneObjectRef oc = *it;

				if (oc != nullptr && oc->_aabb.intersects(p1, p2) && ((renderMask & oc->_renderMask) != 0))
					// Visible, add to list
					result.push_back(oc);
			}

			// Add children to open list if they intersect the region
			if (pCurrent->_hasChildren)
			for (int i = 0; i < 8; i++)
			if (pCurrent->_children[i]->getNumOccupantsBelow() != 0)
				open.push_back(pCurrent->_children[i].get());
		}
	}
}

void Octree::queryPoint(std::vector<SceneObjectRef> &result, const Vec3f &p, unsigned short logicMask, unsigned short renderMask) {
	std::lock_guard<std::recursive_mutex> lock(_mutex);

	// Query outside root elements
	for (std::unordered_set<SceneObjectRef, SceneObjectRef>::iterator it = _outsideRoot.begin(); it != _outsideRoot.end(); it++) {
		SceneObjectRef oc = *it;

		if (oc != nullptr && oc->_aabb.contains(p) && ((renderMask & oc->_renderMask) != 0))
			// Intersects, add to list
			result.push_back(oc);
	}

	std::list<OctreeNode*> open;

	open.push_back(_pRootNode.get());

	while (!open.empty()) {
		// Depth-first (results in less memory usage), remove objects from open list
		OctreeNode* pCurrent = open.back();
		open.pop_back();

		if (pCurrent->_region.contains(p)) {
			for (std::unordered_set<SceneObjectRef, SceneObjectRef>::iterator it = pCurrent->_occupants.begin(); it != pCurrent->_occupants.end(); it++) {
				SceneObjectRef oc = *it;

				if (oc != nullptr && oc->_aabb.contains(p) && ((renderMask & oc->_renderMask) != 0))
					// Visible, add to list
					result.push_back(oc);
			}

			// Add children to open list if they intersect the region
			if (pCurrent->_hasChildren)
			for (int i = 0; i < 8; i++)
			if (pCurrent->_children[i]->getNumOccupantsBelow() != 0)
				open.push_back(pCurrent->_children[i].get());
		}
	}
}

void Octree::queryRay(SceneObjectRef &result, float &t, const Vec3f &start, const Vec3f &direction, unsigned short logicMask, unsigned short renderMask) {
	std::lock_guard<std::recursive_mutex> lock(_mutex);

	result = nullptr;

	t = 999999.0f;

	float t0, t1;

	// Query outside root elements
	for (std::unordered_set<SceneObjectRef, SceneObjectRef>::iterator it = _outsideRoot.begin(); it != _outsideRoot.end(); it++) {
		SceneObjectRef oc = *it;

		if (oc != nullptr && oc->_aabb.intersects(start, direction, t0, t1) && ((renderMask & oc->_renderMask) != 0)) {
			if (t0 < t) {
				result = oc;

				t = t0;
			}
		}
	}

	std::list<OctreeNode*> open;

	open.push_back(_pRootNode.get());

	while (!open.empty()) {
		// Depth-first (results in less memory usage), remove objects from open list
		OctreeNode* pCurrent = open.back();
		open.pop_back();

		if (pCurrent->_region.intersects(start, direction, t0, t1)) {
			for (std::unordered_set<SceneObjectRef, SceneObjectRef>::iterator it = pCurrent->_occupants.begin(); it != pCurrent->_occupants.end(); it++) {
				SceneObjectRef oc = *it;

				if (oc != nullptr && oc->_aabb.intersects(start, direction, t0, t1) && ((renderMask & oc->_renderMask) != 0)) {
					if (t0 < t) {
						result = oc;

						t = t0;
					}
				}
			}

			// Add children to open list if they intersect the region
			if (pCurrent->_hasChildren)
			for (int i = 0; i < 8; i++)
			if (pCurrent->_children[i]->getNumOccupantsBelow() != 0)
				open.push_back(pCurrent->_children[i].get());
		}
	}
}

void Octree::queryRay(SceneObjectRef &result, float &t, const Vec3f &start, const Vec3f &direction, OctreeRayCallBack callBack, unsigned short logicMask, unsigned short renderMask) {
	std::lock_guard<std::recursive_mutex> lock(_mutex);

	result = nullptr;

	t = 999999.0f;

	float t0, t1;

	// Query outside root elements
	for (std::unordered_set<SceneObjectRef, SceneObjectRef>::iterator it = _outsideRoot.begin(); it != _outsideRoot.end(); it++) {
		SceneObjectRef oc = *it;

		if (oc != nullptr && oc->_aabb.intersects(start, direction, t0, t1) && ((renderMask & oc->_renderMask) != 0)) {
			if (t0 < t && callBack(oc, t0, t1)) {
				result = oc;

				t = t0;
			}
		}
	}

	std::list<OctreeNode*> open;

	open.push_back(_pRootNode.get());

	while (!open.empty()) {
		// Depth-first (results in less memory usage), remove objects from open list
		OctreeNode* pCurrent = open.back();
		open.pop_back();

		if (pCurrent->_region.intersects(start, direction, t0, t1)) {
			for (std::unordered_set<SceneObjectRef, SceneObjectRef>::iterator it = pCurrent->_occupants.begin(); it != pCurrent->_occupants.end(); it++) {
				SceneObjectRef oc = *it;

				if (oc != nullptr && oc->_aabb.intersects(start, direction, t0, t1) && ((renderMask & oc->_renderMask) != 0)) {
					if (t0 < t && callBack(oc, t0, t1)) {
						result = oc;

						t = t0;
					}
				}
			}

			// Add children to open list if they intersect the region
			if (pCurrent->_hasChildren)
			for (int i = 0; i < 8; i++)
			if (pCurrent->_children[i]->getNumOccupantsBelow() != 0)
				open.push_back(pCurrent->_children[i].get());
		}
	}
}