#pragma once

#include <pge/geometry/FormTriangle.h>
#include <pge/system/Uncopyable.h>
#include <memory>

namespace pge {
	class BVHNode : public Uncopyable {
	public:
		struct CoordAndIsMin {
			float _coord;
			bool _isMin;

			bool operator<(const CoordAndIsMin &other) const {
				return _coord < other._coord;
			}
		};
	private:
		class BVHTree* _pTree;
		BVHNode* _pParent;

		AABB3D _aabb;

		std::shared_ptr<BVHNode> _pLeft, _pRight;

		unsigned int _numOccupantsBelow;

		std::vector<FormTriangle> _occupants;

		float getCostSAH(const Vec3f &splitPos, int axis);

		// Returns if should split
		bool findSplitPosSAH(float &pos, int &axis, size_t &splitIndex);

	public:
		static const float _traverseCost;
		static const float _intersectCost;

		bool _collapseMarker;

		BVHNode(class BVHTree* pTree, BVHNode* pParent);

		void add(const FormTriangle &triangle, const AABB3D &triangleAABB);
		void split(int numSplitsAfterNoTriangleReduction);

		const AABB3D &getAABB() const {
			return _aabb;
		}

		const BVHNode* getLeft() const {
			return _pLeft.get();
		}

		const BVHNode* getRight() const {
			return _pRight.get();
		}

		const std::vector<FormTriangle> &getTriangles() const {
			return _occupants;
		}

		friend class BVHTree;
	};
}
