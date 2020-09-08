#pragma once

#include "Octree.h"

namespace pge {
	class StaticOctree : public Octree
	{
	public:
		StaticOctree() {}
		StaticOctree(const AABB3D &rootRegion) {
			pRootNode.reset(new OctreeNode(rootRegion, 0, nullptr, this));
		}

		StaticOctree(const StaticOctree &other) {
			*this = other;
		}

		void operator=(const StaticOctree &other) {
			Octree::operator=(other);
		}

		void create(const AABB3D &rootRegion) {
			pRootNode.reset(new OctreeNode(rootRegion, 0, nullptr, this));
		}

		// Inherited from Octree
		void add(const SceneObjectRef &oc);

		void clear() {
			pRootNode.reset();
		}

		const AABB3D &getRootRegion() const {
			return pRootNode->getRegion();
		}

		bool created() const {
			return pRootNode != nullptr;
		}
	};
}