#include "AABB2D.h"
#include "Quaternion.h"

#include <limits>
#include <assert.h>

using namespace pge;

AABB2D::AABB2D()
: lowerBound(0.0f, 0.0f), upperBound(1.0f, 1.0f),
center(0.5f, 0.5f), halfDims(0.5f, 0.5f)
{}

AABB2D::AABB2D(const Vec2f &lowerBound, const Vec2f &upperBound)
: lowerBound(lowerBound), upperBound(upperBound)
{
	calculateHalfDims();
	calculateCenter();
}

void AABB2D::setCenter(const Vec2f &center) {
	this->center = center;

	calculateBounds();
}

void AABB2D::incCenter(const Vec2f &increment) {
	center += increment;

	calculateBounds();
}

void AABB2D::setHalfDims(const Vec2f &halfDims) {
	this->halfDims = halfDims;

	calculateBounds();
}

bool AABB2D::intersects(const AABB2D &other) const {
	if (upperBound.x < other.lowerBound.x)
		return false;
	if (upperBound.y < other.lowerBound.y)
		return false;
	if (lowerBound.x > other.upperBound.x)
		return false;
	if (lowerBound.y > other.upperBound.y)
		return false;

	return true;
}

bool AABB2D::contains(const AABB2D &other) const {
	if (other.lowerBound.x >= lowerBound.x &&
		other.upperBound.x <= upperBound.x &&
		other.lowerBound.y >= lowerBound.y &&
		other.upperBound.y <= upperBound.y)
		return true;

	return false;
}

bool AABB2D::contains(const Vec2f &vec) const {
	if (vec.x >= lowerBound.x &&
		vec.y >= lowerBound.y &&
		vec.x <= upperBound.x &&
		vec.y <= upperBound.y)
		return true;

	return false;
}

AABB2D AABB2D::getTransformedAABB2D(const Matrix4x4f &mat) const {
	AABB2D transformedAABB2D;

	Vec3f newCenter(mat * Vec3f(center.x, center.y, 0.0f));

	transformedAABB2D.lowerBound.x = transformedAABB2D.upperBound.x = newCenter.x;
	transformedAABB2D.lowerBound.y = transformedAABB2D.upperBound.y = newCenter.y;

	// Loop through all corners, transform, and compare
	for (int x = -1; x <= 1; x += 2)
	for (int y = -1; y <= 1; y += 2)
	for (int z = -1; z <= 1; z += 2)
	{
		Vec3f corner(x * halfDims.x + center.x, y * halfDims.y + center.y, 0.0f);

		// Transform the corner
		corner = mat * corner;

		// Compare bounds
		if (corner.x > transformedAABB2D.upperBound.x)
			transformedAABB2D.upperBound.x = corner.x;
		if (corner.y > transformedAABB2D.upperBound.y)
			transformedAABB2D.upperBound.y = corner.y;

		if (corner.x < transformedAABB2D.lowerBound.x)
			transformedAABB2D.lowerBound.x = corner.x;
		if (corner.y < transformedAABB2D.lowerBound.y)
			transformedAABB2D.lowerBound.y = corner.y;
	}

	// Move from local into world space
	transformedAABB2D.calculateHalfDims();
	transformedAABB2D.calculateCenter();

	return transformedAABB2D;
}

float AABB2D::getRadius() const {
	return halfDims.magnitude();
}