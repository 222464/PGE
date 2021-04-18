#include "AABB3D.h"
#include "Quaternion.h"

#include <limits>
#include <assert.h>

using namespace pge;

AABB3D::AABB3D()
: lowerBound(0.0f, 0.0f, 0.0f), upperBound(1.0f, 1.0f, 1.0f),
center(0.5f, 0.5f, 0.5f), halfDims(0.5f, 0.5f, 0.5f)
{}

AABB3D::AABB3D(const Vec3f &lowerBound, const Vec3f &upperBound)
: lowerBound(lowerBound), upperBound(upperBound)
{
    calculateHalfDims();
    calculateCenter();
}

void AABB3D::setCenter(const Vec3f &center) {
    this->center = center;

    calculateBounds();
}

void AABB3D::incCenter(const Vec3f &increment) {
    center += increment;

    calculateBounds();
}

void AABB3D::setHalfDims(const Vec3f &halfDims) {
    this->halfDims = halfDims;

    calculateBounds();
}

bool AABB3D::intersects(const AABB3D &other) const {
    if (upperBound.x < other.lowerBound.x)
        return false;
    if (upperBound.y < other.lowerBound.y)
        return false;
    if (upperBound.z < other.lowerBound.z)
        return false;
    if (lowerBound.x > other.upperBound.x)
        return false;
    if (lowerBound.y > other.upperBound.y)
        return false;
    if (lowerBound.z > other.upperBound.z)
        return false;

    return true;
}

bool AABB3D::intersects(const Vec3f& p1, const Vec3f& p2) const {
    Vec3f d((p2 - p1) * 0.5f);
    Vec3f e((upperBound - lowerBound) * 0.5f);
    Vec3f c(p1 + d - (lowerBound + upperBound) * 0.5f);
    Vec3f ad(fabsf(d.x), fabsf(d.y), fabsf(d.z)); // Returns same vector with all components positive

    if (fabsf(c.x) > e.x + ad.x)
        return false;
    if (fabsf(c.y) > e.y + ad.y)
        return false;
    if (fabsf(c.z) > e.z + ad.z)
        return false;

    float epsilon = std::numeric_limits<float>::epsilon();

    if (fabsf(d.y * c.z - d.z * c.y) > e.y * ad.z + e.z * ad.y + epsilon)
        return false;
    if (fabsf(d.z * c.x - d.x * c.z) > e.z * ad.x + e.x * ad.z + epsilon)
        return false;
    if (fabsf(d.x * c.y - d.y * c.x) > e.x * ad.y + e.y * ad.x + epsilon)
        return false;

    return true;
}

bool AABB3D::intersects(const Vec3f &start, const Vec3f &direction, float &t0, float &t1) const {
    Vec3f directionInv = Vec3f(1.0f, 1.0f, 1.0f) / direction;

    float tx1 = (lowerBound.x - start.x) * directionInv.x;
    float tx2 = (upperBound.x - start.x) * directionInv.x;

    t0 = std::min(tx1, tx2);
    t1 = std::max(tx1, tx2);

    float ty1 = (lowerBound.y - start.y) * directionInv.y;
    float ty2 = (upperBound.y - start.y) * directionInv.y;

    t0 = std::max(t0, std::min(ty1, ty2));
    t1 = std::min(t1, std::max(ty1, ty2));

    float tz1 = (lowerBound.z - start.z) * directionInv.z;
    float tz2 = (upperBound.z - start.z) * directionInv.z;

    t0 = std::max(t0, std::min(tz1, tz2));
    t1 = std::min(t1, std::max(tz1, tz2));

    return t1 >= t0;
}

bool AABB3D::contains(const AABB3D &other) const {
    if (other.lowerBound.x >= lowerBound.x &&
        other.upperBound.x <= upperBound.x &&
        other.lowerBound.y >= lowerBound.y &&
        other.upperBound.y <= upperBound.y &&
        other.lowerBound.z >= lowerBound.z &&
        other.upperBound.z <= upperBound.z)
        return true;

    return false;
}

bool AABB3D::contains(const Vec3f &vec) const {
    if (vec.x >= lowerBound.x &&
        vec.y >= lowerBound.y &&
        vec.z >= lowerBound.z &&
        vec.x <= upperBound.x &&
        vec.y <= upperBound.y &&
        vec.z <= upperBound.z)
        return true;

    return false;
}

AABB3D AABB3D::getTransformedAABB(const Matrix4x4f &mat) const {
    AABB3D transformedAABB;

    Vec3f newCenter(mat * center);

    transformedAABB.lowerBound = newCenter;
    transformedAABB.upperBound = newCenter;

    // Loop through all corners, transform, and compare
    for (int x = -1; x <= 1; x += 2)
    for (int y = -1; y <= 1; y += 2)
    for (int z = -1; z <= 1; z += 2) {
        Vec3f corner(x * halfDims.x + center.x, y * halfDims.y + center.y, z * halfDims.z + center.z);

        // Transform the corner
        corner = mat * corner;

        // Compare bounds
        if (corner.x > transformedAABB.upperBound.x)
            transformedAABB.upperBound.x = corner.x;
        if (corner.y > transformedAABB.upperBound.y)
            transformedAABB.upperBound.y = corner.y;
        if (corner.z > transformedAABB.upperBound.z)
            transformedAABB.upperBound.z = corner.z;

        if (corner.x < transformedAABB.lowerBound.x)
            transformedAABB.lowerBound.x = corner.x;
        if (corner.y < transformedAABB.lowerBound.y)
            transformedAABB.lowerBound.y = corner.y;
        if (corner.z < transformedAABB.lowerBound.z)
            transformedAABB.lowerBound.z = corner.z;
    }

    // Move from local into world space
    transformedAABB.calculateHalfDims();
    transformedAABB.calculateCenter();

    return transformedAABB;
}

Vec3f AABB3D::getVertexP(const Vec3f &normal) const {
    Vec3f p(lowerBound);

    if (normal.x >= 0.0f)
        p.x = upperBound.x;
    if (normal.y >= 0.0f)
        p.y = upperBound.y;
    if (normal.z >= 0.0f)
        p.z = upperBound.z;

    return p;
}

Vec3f AABB3D::getVertexN(const Vec3f &normal) const {
    Vec3f n(upperBound);

    if (normal.x >= 0.0f)
        n.x = lowerBound.x;
    if (normal.y >= 0.0f)
        n.y = lowerBound.y;
    if (normal.z >= 0.0f)
        n.z = lowerBound.z;

    return n;
}

float AABB3D::getRadius() const {
    return halfDims.magnitude();
}

void AABB3D::expand(const Vec3f &point) {
    if (point.x < lowerBound.x)
        lowerBound.x = point.x;
    if (point.y < lowerBound.y)
        lowerBound.y = point.y;
    if (point.z < lowerBound.z)
        lowerBound.z = point.z;
    if (point.x > upperBound.x)
        upperBound.x = point.x;
    if (point.y > upperBound.y)
        upperBound.y = point.y;
    if (point.z > upperBound.z)
        upperBound.z = point.z;
}

void AABB3D::expand(const AABB3D &other) {
    if (other.lowerBound.x < lowerBound.x)
        lowerBound.x = other.lowerBound.x;
    if (other.lowerBound.y < lowerBound.y)
        lowerBound.y = other.lowerBound.y;
    if (other.lowerBound.z < lowerBound.z)
        lowerBound.z = other.lowerBound.z;
    if (other.upperBound.x > upperBound.x)
        upperBound.x = other.upperBound.x;
    if (other.upperBound.y > upperBound.y)
        upperBound.y = other.upperBound.y;
    if (other.upperBound.z > upperBound.z)
        upperBound.z = other.upperBound.z;
}