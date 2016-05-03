#pragma once

#include <pge/rendering/octree/Octree.h>

namespace pge {
	class StaticOctree : public Octree
	{
	public:
		StaticOctree() {}
		StaticOctree(const AABB3D &rootRegion) {
			_pRootNode.reset(new OctreeNode(rootRegion, 0, nullptr, this));
		}

		StaticOctree(const StaticOctree &other) {
			*this = other;
		}

		void operator=(const StaticOctree &other) {
			Octree::operator=(other);
		}

		void create(const AABB3D &rootRegion) {
			_pRootNode.reset(new OctreeNode(rootRegion, 0, nullptr, this));
		}

		// Inherited from Octree
		void add(const SceneObjectRef &oc);

		void clear() {
			_pRootNode.reset();
		}

		const AABB3D &getRootRegion() const {
			return _pRootNode->getRegion();
		}

		bool created() const {
			return _pRootNode != nullptr;
		}
	};
}