#include <pge/bvh/BVHTree.h>

#include <assert.h>

using namespace pge;

BVHTree::BVHTree()
: _pRootNode(nullptr), _maxSplitsAfterNoTriangleReduction(3)
{}

void BVHTree::create(const AABB3D &rootRegion) {
	_pRootNode.reset(new BVHNode(this, nullptr));
	_pRootNode->_aabb = rootRegion;
}

void BVHTree::add(FormTriangle &triangle) {
	assert(_pRootNode != nullptr);

	_pRootNode->add(triangle, triangle.getAABB());
}

bool BVHTree::rayTrace(const Vec3f &origin, const Vec3f &direction,
	FormTriangle &triangle, Vec3f &point, Vec3f &normal)
{
	// Implement me!

	return false;
}
