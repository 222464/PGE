#include <pge/constructs/AABB3D.h>
#include <pge/constructs/Quaternion.h>

#include <limits>
#include <assert.h>

using namespace pge;

AABB3D::AABB3D()
: _lowerBound(0.0f, 0.0f, 0.0f), _upperBound(1.0f, 1.0f, 1.0f),
_center(0.5f, 0.5f, 0.5f), _halfDims(0.5f, 0.5f, 0.5f)
{}

AABB3D::AABB3D(const Vec3f &lowerBound, const Vec3f &upperBound)
: _lowerBound(lowerBound), _upperBound(upperBound)
{
	calculateHalfDims();
	calculateCenter();
}

void AABB3D::setCenter(const Vec3f &center) {
	_center = center;

	calculateBounds();
}

void AABB3D::incCenter(const Vec3f &increment) {
	_center += increment;

	calculateBounds();
}

void AABB3D::setHalfDims(const Vec3f &halfDims) {
	_halfDims = halfDims;

	calculateBounds();
}

bool AABB3D::intersects(const AABB3D &other) const {
	if (_upperBound.x < other._lowerBound.x)
		return false;
	if (_upperBound.y < other._lowerBound.y)
		return false;
	if (_upperBound.z < other._lowerBound.z)
		return false;
	if (_lowerBound.x > other._upperBound.x)
		return false;
	if (_lowerBound.y > other._upperBound.y)
		return false;
	if (_lowerBound.z > other._upperBound.z)
		return false;

	return true;
}

bool AABB3D::intersects(const Vec3f& p1, const Vec3f& p2) const {
	Vec3f d((p2 - p1) * 0.5f);
	Vec3f e((_upperBound - _lowerBound) * 0.5f);
	Vec3f c(p1 + d - (_lowerBound + _upperBound) * 0.5f);
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

	float tx1 = (_lowerBound.x - start.x) * directionInv.x;
	float tx2 = (_upperBound.x - start.x) * directionInv.x;

	t0 = std::min(tx1, tx2);
	t1 = std::max(tx1, tx2);

	float ty1 = (_lowerBound.y - start.y) * directionInv.y;
	float ty2 = (_upperBound.y - start.y) * directionInv.y;

	t0 = std::max(t0, std::min(ty1, ty2));
	t1 = std::min(t1, std::max(ty1, ty2));

	float tz1 = (_lowerBound.z - start.z) * directionInv.z;
	float tz2 = (_upperBound.z - start.z) * directionInv.z;

	t0 = std::max(t0, std::min(tz1, tz2));
	t1 = std::min(t1, std::max(tz1, tz2));

	return t1 >= t0;
}

bool AABB3D::contains(const AABB3D &other) const {
	if (other._lowerBound.x >= _lowerBound.x &&
		other._upperBound.x <= _upperBound.x &&
		other._lowerBound.y >= _lowerBound.y &&
		other._upperBound.y <= _upperBound.y &&
		other._lowerBound.z >= _lowerBound.z &&
		other._upperBound.z <= _upperBound.z)
		return true;

	return false;
}

bool AABB3D::contains(const Vec3f &vec) const {
	if (vec.x >= _lowerBound.x &&
		vec.y >= _lowerBound.y &&
		vec.z >= _lowerBound.z &&
		vec.x <= _upperBound.x &&
		vec.y <= _upperBound.y &&
		vec.z <= _upperBound.z)
		return true;

	return false;
}

AABB3D AABB3D::getTransformedAABB(const Matrix4x4f &mat) const {
	AABB3D transformedAABB;

	Vec3f newCenter(mat * _center);

	transformedAABB._lowerBound = newCenter;
	transformedAABB._upperBound = newCenter;

	// Loop through all corners, transform, and compare
	for (int x = -1; x <= 1; x += 2)
	for (int y = -1; y <= 1; y += 2)
	for (int z = -1; z <= 1; z += 2) {
		Vec3f corner(x * _halfDims.x + _center.x, y * _halfDims.y + _center.y, z * _halfDims.z + _center.z);

		// Transform the corner
		corner = mat * corner;

		// Compare bounds
		if (corner.x > transformedAABB._upperBound.x)
			transformedAABB._upperBound.x = corner.x;
		if (corner.y > transformedAABB._upperBound.y)
			transformedAABB._upperBound.y = corner.y;
		if (corner.z > transformedAABB._upperBound.z)
			transformedAABB._upperBound.z = corner.z;

		if (corner.x < transformedAABB._lowerBound.x)
			transformedAABB._lowerBound.x = corner.x;
		if (corner.y < transformedAABB._lowerBound.y)
			transformedAABB._lowerBound.y = corner.y;
		if (corner.z < transformedAABB._lowerBound.z)
			transformedAABB._lowerBound.z = corner.z;
	}

	// Move from local into world space
	transformedAABB.calculateHalfDims();
	transformedAABB.calculateCenter();

	return transformedAABB;
}

Vec3f AABB3D::getVertexP(const Vec3f &normal) const {
	Vec3f p(_lowerBound);

	if (normal.x >= 0.0f)
		p.x = _upperBound.x;
	if (normal.y >= 0.0f)
		p.y = _upperBound.y;
	if (normal.z >= 0.0f)
		p.z = _upperBound.z;

	return p;
}

Vec3f AABB3D::getVertexN(const Vec3f &normal) const {
	Vec3f n(_upperBound);

	if (normal.x >= 0.0f)
		n.x = _lowerBound.x;
	if (normal.y >= 0.0f)
		n.y = _lowerBound.y;
	if (normal.z >= 0.0f)
		n.z = _lowerBound.z;

	return n;
}

float AABB3D::getRadius() const {
	return _halfDims.magnitude();
}

void AABB3D::expand(const Vec3f &point) {
	if (point.x < _lowerBound.x)
		_lowerBound.x = point.x;
	if (point.y < _lowerBound.y)
		_lowerBound.y = point.y;
	if (point.z < _lowerBound.z)
		_lowerBound.z = point.z;
	if (point.x > _upperBound.x)
		_upperBound.x = point.x;
	if (point.y > _upperBound.y)
		_upperBound.y = point.y;
	if (point.z > _upperBound.z)
		_upperBound.z = point.z;
}

void AABB3D::expand(const AABB3D &other) {
	if (other._lowerBound.x < _lowerBound.x)
		_lowerBound.x = other._lowerBound.x;
	if (other._lowerBound.y < _lowerBound.y)
		_lowerBound.y = other._lowerBound.y;
	if (other._lowerBound.z < _lowerBound.z)
		_lowerBound.z = other._lowerBound.z;
	if (other._upperBound.x > _upperBound.x)
		_upperBound.x = other._upperBound.x;
	if (other._upperBound.y > _upperBound.y)
		_upperBound.y = other._upperBound.y;
	if (other._upperBound.z > _upperBound.z)
		_upperBound.z = other._upperBound.z;
}