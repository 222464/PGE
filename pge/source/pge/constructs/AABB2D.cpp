#include <pge/constructs/AABB2D.h>
#include <pge/constructs/Quaternion.h>

#include <limits>
#include <assert.h>

using namespace pge;

AABB2D::AABB2D()
: _lowerBound(0.0f, 0.0f), _upperBound(1.0f, 1.0f),
_center(0.5f, 0.5f), _halfDims(0.5f, 0.5f)
{}

AABB2D::AABB2D(const Vec2f &lowerBound, const Vec2f &upperBound)
: _lowerBound(lowerBound), _upperBound(upperBound)
{
	calculateHalfDims();
	calculateCenter();
}

void AABB2D::setCenter(const Vec2f &center) {
	_center = center;

	calculateBounds();
}

void AABB2D::incCenter(const Vec2f &increment) {
	_center += increment;

	calculateBounds();
}

void AABB2D::setHalfDims(const Vec2f &halfDims) {
	_halfDims = halfDims;

	calculateBounds();
}

bool AABB2D::intersects(const AABB2D &other) const {
	if (_upperBound.x < other._lowerBound.x)
		return false;
	if (_upperBound.y < other._lowerBound.y)
		return false;
	if (_lowerBound.x > other._upperBound.x)
		return false;
	if (_lowerBound.y > other._upperBound.y)
		return false;

	return true;
}

bool AABB2D::contains(const AABB2D &other) const {
	if (other._lowerBound.x >= _lowerBound.x &&
		other._upperBound.x <= _upperBound.x &&
		other._lowerBound.y >= _lowerBound.y &&
		other._upperBound.y <= _upperBound.y)
		return true;

	return false;
}

bool AABB2D::contains(const Vec2f &vec) const {
	if (vec.x >= _lowerBound.x &&
		vec.y >= _lowerBound.y &&
		vec.x <= _upperBound.x &&
		vec.y <= _upperBound.y)
		return true;

	return false;
}

AABB2D AABB2D::getTransformedAABB2D(const Matrix4x4f &mat) const {
	AABB2D transformedAABB2D;

	Vec3f newCenter(mat * Vec3f(_center.x, _center.y, 0.0f));

	transformedAABB2D._lowerBound.x = transformedAABB2D._upperBound.x = newCenter.x;
	transformedAABB2D._lowerBound.y = transformedAABB2D._upperBound.y = newCenter.y;

	// Loop through all corners, transform, and compare
	for (int x = -1; x <= 1; x += 2)
	for (int y = -1; y <= 1; y += 2)
	for (int z = -1; z <= 1; z += 2)
	{
		Vec3f corner(x * _halfDims.x + _center.x, y * _halfDims.y + _center.y, 0.0f);

		// Transform the corner
		corner = mat * corner;

		// Compare bounds
		if (corner.x > transformedAABB2D._upperBound.x)
			transformedAABB2D._upperBound.x = corner.x;
		if (corner.y > transformedAABB2D._upperBound.y)
			transformedAABB2D._upperBound.y = corner.y;

		if (corner.x < transformedAABB2D._lowerBound.x)
			transformedAABB2D._lowerBound.x = corner.x;
		if (corner.y < transformedAABB2D._lowerBound.y)
			transformedAABB2D._lowerBound.y = corner.y;
	}

	// Move from local into world space
	transformedAABB2D.calculateHalfDims();
	transformedAABB2D.calculateCenter();

	return transformedAABB2D;
}

float AABB2D::getRadius() const {
	return _halfDims.magnitude();
}