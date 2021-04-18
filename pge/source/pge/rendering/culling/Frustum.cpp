#include "Frustum.h"

#include <assert.h>

using namespace pge;

void Frustum::extractFromMatrix(const Matrix4x4f &camera) {
    planes[planeNear].normalizedFromEquationCoeffs(
        camera.get(0, 2) + camera.get(0, 3),
        camera.get(1, 2) + camera.get(1, 3),
        camera.get(2, 2) + camera.get(2, 3),
        camera.get(3, 2) + camera.get(3, 3));

    planes[planeFar].normalizedFromEquationCoeffs(
        -camera.get(0, 2) + camera.get(0, 3),
        -camera.get(1, 2) + camera.get(1, 3),
        -camera.get(2, 2) + camera.get(2, 3),
        -camera.get(3, 2) + camera.get(3, 3));

    planes[planeBottom].normalizedFromEquationCoeffs(
        camera.get(0, 1) + camera.get(0, 3),
        camera.get(1, 1) + camera.get(1, 3),
        camera.get(2, 1) + camera.get(2, 3),
        camera.get(3, 1) + camera.get(3, 3));

    planes[planeTop].normalizedFromEquationCoeffs(
        -camera.get(0, 1) + camera.get(0, 3),
        -camera.get(1, 1) + camera.get(1, 3),
        -camera.get(2, 1) + camera.get(2, 3),
        -camera.get(3, 1) + camera.get(3, 3));

    planes[planeLeft].normalizedFromEquationCoeffs(
        camera.get(0, 0) + camera.get(0, 3),
        camera.get(1, 0) + camera.get(1, 3),
        camera.get(2, 0) + camera.get(2, 3),
        camera.get(3, 0) + camera.get(3, 3));

    planes[planeRight].normalizedFromEquationCoeffs(
        -camera.get(0, 0) + camera.get(0, 3),
        -camera.get(1, 0) + camera.get(1, 3),
        -camera.get(2, 0) + camera.get(2, 3),
        -camera.get(3, 0) + camera.get(3, 3));
}

Frustum::ObjectLocation Frustum::testAABB(const AABB3D &aabb) const {
    ObjectLocation location = inside;

    // For each plane
    for(unsigned char p = 0; p < 6; p++) {
        // If positive vertex is outside
        if(planes[p].signedDistanceTo(aabb.getVertexP(Vec3f(planes[p].a, planes[p].b, planes[p].c))) < 0.0f)
            return outside;

        // If positive vertex is inside
        else if(planes[p].signedDistanceTo(aabb.getVertexN(Vec3f(planes[p].a, planes[p].b, planes[p].c))) < 0.0f)
            location = intersect;
    }

    return location;
}

bool Frustum::testAABBOutside(const AABB3D &aabb) const {
    // For each plane
    for(unsigned char p = 0; p < 6; p++) {
        // If positive vertex is outside
        if(planes[p].signedDistanceTo(aabb.getVertexP(Vec3f(planes[p].a, planes[p].b, planes[p].c))) < 0.0f)
            return true;
    }

    return false;
}

void Frustum::calculateCorners(const Matrix4x4f &cameraInverse) {
    size_t ci = 0;

    for(int x = -1; x < 2; x += 2)
        for(int y = -1; y < 2; y += 2)
            for(int z = -1; z < 2; z += 2) {
                Vec4f clipSpaceCoord(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), 1.0f);

                assert(ci < 8);

                Vec4f homogenous(cameraInverse * clipSpaceCoord);

                float wInv = 1.0f / homogenous.w;

                corners[ci] = Vec3f(homogenous.x * wInv, homogenous.y * wInv, homogenous.z * wInv);

                ci++;
            }
}