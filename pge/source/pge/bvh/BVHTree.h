#pragma once

#include <pge/bvh/BVHNode.h>
#include <pge/system/Uncopyable.h>

#include <pge/constructs/AABB3D.h>

#include <pge/geometry/FormTriangle.h>

#include <array>

namespace pge {
	class BVHTree : public Uncopyable {
	private:
		std::shared_ptr<BVHNode> _pRootNode;

	public:
		unsigned int _maxSplitsAfterNoTriangleReduction;

		BVHTree();

		void create(const AABB3D &rootRegion);
		void add(FormTriangle &triangle);

		void compile() {
			assert(_pRootNode != nullptr);

			// TODO: Remove useless (empty) nodes
			_pRootNode->split(0);
		}

		bool rayTrace(const Vec3f &origin, const Vec3f &direction,
			FormTriangle &triangle, Vec3f &point, Vec3f &normal);

		const BVHNode* getRootNode() const {
			return _pRootNode.get();
		}

		friend BVHNode;
	};
}