#pragma once

#include <pge/rendering/octree/Octree.h>

namespace pge {
	class DynamicOctree : public Octree {
	private:
		void expand();
		void contract();

	public:
		size_t _minOutsideRoot;
		size_t _maxOutsideRoot;

		DynamicOctree()
			: _minOutsideRoot(1), _maxOutsideRoot(8)
		{}

		DynamicOctree(const AABB3D &rootRegion)
			: _minOutsideRoot(1), _maxOutsideRoot(8)
		{
			_pRootNode.reset(new OctreeNode(rootRegion, 0, nullptr, this));
		}

		DynamicOctree(const DynamicOctree &other) {
			*this = other;
		}

		void operator=(const DynamicOctree &other);

		void create(const AABB3D &rootRegion) {
			_pRootNode.reset(new OctreeNode(rootRegion, 0, nullptr, this));
		}

		// Inherited from Octree
		void add(const SceneObjectRef &oc);

		void clear() {
			_pRootNode.reset();
		}

		// Resizes Octree
		void trim();

		bool created() const {
			return _pRootNode != nullptr;
		}

		const AABB3D &getRootRegion() const {
			return _pRootNode->getRegion();
		}
	};
}