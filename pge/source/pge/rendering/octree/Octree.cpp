#include "Octree.h"

#include "../../scene/SceneObject.h"

#include <algorithm>

#include <assert.h>

using namespace pge;

Octree::Octree()
: minNumNodeOccupants(3),
maxNumNodeOccupants(6),
maxLevels(40),
oversizeMultiplier(1.0f)
{}

void Octree::operator=(const Octree &other) {
	minNumNodeOccupants = other.minNumNodeOccupants;
	maxNumNodeOccupants = other.maxNumNodeOccupants;
	maxLevels = other.maxLevels;
	oversizeMultiplier = other.oversizeMultiplier;

	outsideRoot = other.outsideRoot;

	if (other.pRootNode != nullptr) {
		pRootNode.reset(new OctreeNode());

		recursiveCopy(pRootNode.get(), other.pRootNode.get(), nullptr);
	}
}

void Octree::setOctree(const SceneObjectRef &oc) {
	oc->pOctree = this;
}

void Octree::recursiveCopy(OctreeNode* pThisNode, OctreeNode* pOtherNode, OctreeNode* pThisParent) {
	pThisNode->hasChildren = pOtherNode->hasChildren;
	pThisNode->level = pOtherNode->level;
	pThisNode->numOccupantsBelow = pOtherNode->numOccupantsBelow;
	pThisNode->occupants = pOtherNode->occupants;
	pThisNode->region = pOtherNode->region;

	pThisNode->pParent = pThisParent;

	pThisNode->pOctree = this;

	if (pThisNode->hasChildren)
	for (int i = 0; i < 8; i++) {
		pThisNode->children[i].reset(new OctreeNode());

		recursiveCopy(pThisNode->children[i].get(), pOtherNode->children[i].get(), pThisNode);
	}
}

void Octree::pruneDeadReferences() {
	for (std::unordered_set<SceneObjectRef, SceneObjectRef>::iterator it = outsideRoot.begin(); it != outsideRoot.end();)
	if (it->isAlive())
		it++;
	else
		it = outsideRoot.erase(it);

	if (pRootNode != nullptr)
		pRootNode->pruneDeadReferences();
}

void Octree::queryRegion(const AABB3D &region, std::vector<SceneObjectRef> &result, unsigned short logicMask, unsigned short renderMask) {
	std::lock_guard<std::recursive_mutex> lock(mutex);

	// Query outside root elements
	for (std::unordered_set<SceneObjectRef, SceneObjectRef>::iterator it = outsideRoot.begin(); it != outsideRoot.end(); it++) {
		SceneObjectRef oc = *it;

		if (oc != nullptr && region.intersects(oc->aabb) && ((renderMask & oc->renderMask) != 0))
			// Intersects, add to list
			result.push_back(oc);
	}

	std::list<OctreeNode*> open;

	open.push_back(pRootNode.get());

	while (!open.empty()) {
		// Depth-first (results in less memory usage), remove objects from open list
		OctreeNode* pCurrent = open.back();
		open.pop_back();

		if (region.intersects(pCurrent->region)) {
			for (std::unordered_set<SceneObjectRef, SceneObjectRef>::iterator it = pCurrent->occupants.begin(); it != pCurrent->occupants.end(); it++) {
				SceneObjectRef oc = *it;

				if (oc != nullptr && region.intersects(oc->aabb) && ((renderMask & oc->renderMask) != 0))
					// Visible, add to list
					result.push_back(oc);
			}

			// Add children to open list if they intersect the region
			if (pCurrent->hasChildren)
			for (int i = 0; i < 8; i++)
			if (pCurrent->children[i]->getNumOccupantsBelow() != 0)
				open.push_back(pCurrent->children[i].get());
		}
	}
}

void Octree::queryFrustum(std::vector<SceneObjectRef> &result, const Frustum &frustum, unsigned short logicMask, unsigned short renderMask) {
	std::lock_guard<std::recursive_mutex> lock(mutex);

	// Query outside root elements - add them if they are visible
	for (std::unordered_set<SceneObjectRef, SceneObjectRef>::iterator it = outsideRoot.begin(); it != outsideRoot.end(); it++) {
		SceneObjectRef oc = *it;

		if (oc != nullptr && !frustum.testAABBOutside(oc->aabb) && ((renderMask & oc->renderMask) != 0))
			// Visible, add to list
			result.push_back(oc);
	}

	std::list<OctreeNode*> open;

	open.push_back(pRootNode.get());

	while (!open.empty()) {
		// Depth-first (results in less memory usage), remove objects from open list
		OctreeNode* pCurrent = open.back();
		open.pop_back();

		switch (frustum.testAABB(pCurrent->region)) {
		case Frustum::inside:
			// Add all of this nodes occupants and those below, since all must be visibile
			pCurrent->getAllOccupantsBelow(result);

			break;

		case Frustum::intersect:
			// Add occupants if they are visible
			for (std::unordered_set<SceneObjectRef, SceneObjectRef>::iterator it = pCurrent->occupants.begin(); it != pCurrent->occupants.end(); it++) {
				SceneObjectRef oc = *it;

				if (oc != nullptr && !frustum.testAABBOutside(oc->aabb) && ((renderMask & oc->renderMask) != 0))
					// Visible, add to list
					result.push_back(oc);
			}

			// Add children to open list
			if (pCurrent->hasChildren)
			for (int i = 0; i < 8; i++)
			if (pCurrent->children[i]->getNumOccupantsBelow() != 0)
				open.push_back(pCurrent->children[i].get());

			break;

			// Outside case is ignored
		}
	}
}

void Octree::querySegment(std::vector<SceneObjectRef> &result, const Vec3f &p1, const Vec3f &p2, unsigned short logicMask, unsigned short renderMask) {
	std::lock_guard<std::recursive_mutex> lock(mutex);

	// Query outside root elements
	for (std::unordered_set<SceneObjectRef, SceneObjectRef>::iterator it = outsideRoot.begin(); it != outsideRoot.end(); it++) {
		SceneObjectRef oc = *it;

		if (oc != nullptr && oc->aabb.intersects(p1, p2) && ((renderMask & oc->renderMask) != 0))
			// Intersects, add to list
			result.push_back(oc);
	}

	std::list<OctreeNode*> open;

	open.push_back(pRootNode.get());

	while (!open.empty()) {
		// Depth-first (results in less memory usage), remove objects from open list
		OctreeNode* pCurrent = open.back();
		open.pop_back();

		if (pCurrent->region.intersects(p1, p2)) {
			for (std::unordered_set<SceneObjectRef, SceneObjectRef>::iterator it = pCurrent->occupants.begin(); it != pCurrent->occupants.end(); it++) {
				SceneObjectRef oc = *it;

				if (oc != nullptr && oc->aabb.intersects(p1, p2) && ((renderMask & oc->renderMask) != 0))
					// Visible, add to list
					result.push_back(oc);
			}

			// Add children to open list if they intersect the region
			if (pCurrent->hasChildren)
			for (int i = 0; i < 8; i++)
			if (pCurrent->children[i]->getNumOccupantsBelow() != 0)
				open.push_back(pCurrent->children[i].get());
		}
	}
}

void Octree::queryPoint(std::vector<SceneObjectRef> &result, const Vec3f &p, unsigned short logicMask, unsigned short renderMask) {
	std::lock_guard<std::recursive_mutex> lock(mutex);

	// Query outside root elements
	for (std::unordered_set<SceneObjectRef, SceneObjectRef>::iterator it = outsideRoot.begin(); it != outsideRoot.end(); it++) {
		SceneObjectRef oc = *it;

		if (oc != nullptr && oc->aabb.contains(p) && ((renderMask & oc->renderMask) != 0))
			// Intersects, add to list
			result.push_back(oc);
	}

	std::list<OctreeNode*> open;

	open.push_back(pRootNode.get());

	while (!open.empty()) {
		// Depth-first (results in less memory usage), remove objects from open list
		OctreeNode* pCurrent = open.back();
		open.pop_back();

		if (pCurrent->region.contains(p)) {
			for (std::unordered_set<SceneObjectRef, SceneObjectRef>::iterator it = pCurrent->occupants.begin(); it != pCurrent->occupants.end(); it++) {
				SceneObjectRef oc = *it;

				if (oc != nullptr && oc->aabb.contains(p) && ((renderMask & oc->renderMask) != 0))
					// Visible, add to list
					result.push_back(oc);
			}

			// Add children to open list if they intersect the region
			if (pCurrent->hasChildren)
			for (int i = 0; i < 8; i++)
			if (pCurrent->children[i]->getNumOccupantsBelow() != 0)
				open.push_back(pCurrent->children[i].get());
		}
	}
}

void Octree::queryRay(SceneObjectRef &result, float &t, const Vec3f &start, const Vec3f &direction, unsigned short logicMask, unsigned short renderMask) {
	std::lock_guard<std::recursive_mutex> lock(mutex);

	result = nullptr;

	t = 999999.0f;

	float t0, t1;

	// Query outside root elements
	for (std::unordered_set<SceneObjectRef, SceneObjectRef>::iterator it = outsideRoot.begin(); it != outsideRoot.end(); it++) {
		SceneObjectRef oc = *it;

		if (oc != nullptr && oc->aabb.intersects(start, direction, t0, t1) && ((renderMask & oc->renderMask) != 0)) {
			if (t0 < t) {
				result = oc;

				t = t0;
			}
		}
	}

	std::list<OctreeNode*> open;

	open.push_back(pRootNode.get());

	while (!open.empty()) {
		// Depth-first (results in less memory usage), remove objects from open list
		OctreeNode* pCurrent = open.back();
		open.pop_back();

		if (pCurrent->region.intersects(start, direction, t0, t1)) {
			for (std::unordered_set<SceneObjectRef, SceneObjectRef>::iterator it = pCurrent->occupants.begin(); it != pCurrent->occupants.end(); it++) {
				SceneObjectRef oc = *it;

				if (oc != nullptr && oc->aabb.intersects(start, direction, t0, t1) && ((renderMask & oc->renderMask) != 0)) {
					if (t0 < t) {
						result = oc;

						t = t0;
					}
				}
			}

			// Add children to open list if they intersect the region
			if (pCurrent->hasChildren)
			for (int i = 0; i < 8; i++)
			if (pCurrent->children[i]->getNumOccupantsBelow() != 0)
				open.push_back(pCurrent->children[i].get());
		}
	}
}

void Octree::queryRay(SceneObjectRef &result, float &t, const Vec3f &start, const Vec3f &direction, OctreeRayCallBack callBack, unsigned short logicMask, unsigned short renderMask) {
	std::lock_guard<std::recursive_mutex> lock(mutex);

	result = nullptr;

	t = 999999.0f;

	float t0, t1;

	// Query outside root elements
	for (std::unordered_set<SceneObjectRef, SceneObjectRef>::iterator it = outsideRoot.begin(); it != outsideRoot.end(); it++) {
		SceneObjectRef oc = *it;

		if (oc != nullptr && oc->aabb.intersects(start, direction, t0, t1) && ((renderMask & oc->renderMask) != 0)) {
			if (t0 < t && callBack(oc, t0, t1)) {
				result = oc;

				t = t0;
			}
		}
	}

	std::list<OctreeNode*> open;

	open.push_back(pRootNode.get());

	while (!open.empty()) {
		// Depth-first (results in less memory usage), remove objects from open list
		OctreeNode* pCurrent = open.back();
		open.pop_back();

		if (pCurrent->region.intersects(start, direction, t0, t1)) {
			for (std::unordered_set<SceneObjectRef, SceneObjectRef>::iterator it = pCurrent->occupants.begin(); it != pCurrent->occupants.end(); it++) {
				SceneObjectRef oc = *it;

				if (oc != nullptr && oc->aabb.intersects(start, direction, t0, t1) && ((renderMask & oc->renderMask) != 0)) {
					if (t0 < t && callBack(oc, t0, t1)) {
						result = oc;

						t = t0;
					}
				}
			}

			// Add children to open list if they intersect the region
			if (pCurrent->hasChildren)
			for (int i = 0; i < 8; i++)
			if (pCurrent->children[i]->getNumOccupantsBelow() != 0)
				open.push_back(pCurrent->children[i].get());
		}
	}
}