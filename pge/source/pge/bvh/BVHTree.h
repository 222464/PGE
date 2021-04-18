#pragma once

#include "BVHNode.h"
#include "../system/Uncopyable.h"

#include "../constructs/AABB3D.h"

#include "../geometry/FormTriangle.h"

#include <array>

namespace pge {
    class BVHTree : public Uncopyable {
    private:
        std::shared_ptr<BVHNode> pRootNode;

    public:
        unsigned int maxSplitsAfterNoTriangleReduction;

        BVHTree();

        void create(const AABB3D &rootRegion);
        void add(FormTriangle &triangle);

        void compile() {
            assert(pRootNode != nullptr);

            // TODO: Remove useless (empty) nodes
            pRootNode->split(0);
        }

        bool rayTrace(const Vec3f &origin, const Vec3f &direction,
            FormTriangle &triangle, Vec3f &point, Vec3f &normal);

        const BVHNode* getRootNode() const {
            return pRootNode.get();
        }

        friend BVHNode;
    };
}