#pragma once

#include "Octree.h"

namespace pge {
	class DynamicOctree : public Octree {
	private:
		void expand();
		void contract();

	public:
		size_t minOutsideRoot;
		size_t maxOutsideRoot;

		DynamicOctree()
			: minOutsideRoot(1), maxOutsideRoot(8)
		{}

		DynamicOctree(const AABB3D &rootRegion)
			: minOutsideRoot(1), maxOutsideRoot(8)
		{
			pRootNode.reset(new OctreeNode(rootRegion, 0, nullptr, this));
		}

		DynamicOctree(const DynamicOctree &other) {
			*this = other;
		}

		void operator=(const DynamicOctree &other);

		void create(const AABB3D &rootRegion) {
			pRootNode.reset(new OctreeNode(rootRegion, 0, nullptr, this));
		}

		// Inherited from Octree
		void add(const SceneObjectRef &oc);

		void clear() {
			pRootNode.reset();
		}

		// Resizes Octree
		void trim();

		bool created() const {
			return pRootNode != nullptr;
		}

		const AABB3D &getRootRegion() const {
			return pRootNode->getRegion();
		}
	};
}