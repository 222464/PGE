#pragma once

#include "../geometry/FormTriangle.h"
#include "../system/Uncopyable.h"
#include <memory>

namespace pge {
    class BVHNode : public Uncopyable {
    public:
        struct CoordAndIsMin {
            float coord;
            bool isMin;

            bool operator<(const CoordAndIsMin &other) const {
                return coord < other.coord;
            }
        };
    private:
        class BVHTree* pTree;
        BVHNode* pParent;

        AABB3D aabb;

        std::shared_ptr<BVHNode> pLeft, pRight;

        unsigned int numOccupantsBelow;

        std::vector<FormTriangle> occupants;

        float getCostSAH(const Vec3f &splitPos, int axis);

        // Returns if should split
        bool findSplitPosSAH(float &pos, int &axis, size_t &splitIndex);

    public:
        static const float traverseCost;
        static const float intersectCost;

        bool collapseMarker;

        BVHNode(class BVHTree* pTree, BVHNode* pParent);

        void add(const FormTriangle &triangle, const AABB3D &triangleAABB);
        void split(int numSplitsAfterNoTriangleReduction);

        const AABB3D &getAABB() const {
            return aabb;
        }

        const BVHNode* getLeft() const {
            return pLeft.get();
        }

        const BVHNode* getRight() const {
            return pRight.get();
        }

        const std::vector<FormTriangle> &getTriangles() const {
            return occupants;
        }

        friend class BVHTree;
    };
}
