#pragma once

#include "../../constructs/Vec3f.h"
#include "../../constructs/AABB3D.h"
#include "../../constructs/PlaneEquationf.h"
#include "../../constructs/Matrix4x4f.h"

namespace pge {
    class Frustum {
    private:
        enum PlaneOrientation {
            planeTop = 0, planeBottom, planeLeft, planeRight, planeNear, planeFar
        };

        std::array<PlaneEquationf, 6> planes;

        std::array<Vec3f, 8> corners;

    public:
        enum ObjectLocation {
            outside, intersect, inside
        };

        void extractFromMatrix(const Matrix4x4f &camera); // ViewProjection

        // Tests
        ObjectLocation testAABB(const AABB3D &aabb) const;
        bool testAABBOutside(const AABB3D &aabb) const;

        void calculateCorners(const Matrix4x4f &cameraInverse); // Inverse ViewProjection

        Vec3f getCorner(size_t index) const {
            return corners[index];
        }
    };
}