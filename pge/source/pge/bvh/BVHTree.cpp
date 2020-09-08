#include "BVHTree.h"

#include <assert.h>

using namespace pge;

BVHTree::BVHTree()
: pRootNode(nullptr), maxSplitsAfterNoTriangleReduction(3)
{}

void BVHTree::create(const AABB3D &rootRegion) {
	pRootNode.reset(new BVHNode(this, nullptr));
	pRootNode->aabb = rootRegion;
}

void BVHTree::add(FormTriangle &triangle) {
	assert(pRootNode != nullptr);

	pRootNode->add(triangle, triangle.getAABB());
}

bool BVHTree::rayTrace(const Vec3f &origin, const Vec3f &direction,
	FormTriangle &triangle, Vec3f &point, Vec3f &normal)
{
	// Implement me!

	return false;
}
