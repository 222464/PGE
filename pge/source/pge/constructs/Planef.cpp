#include <pge/constructs/Planef.h>

using namespace pge;

void Planef::fromPoints(const Vec3f &p1, const Vec3f &p2, const Vec3f &p3) {
	// Set anchor to any of the points
	_point = p1;

	// Calculate the normal
	_normal = (p2 - p1).cross(p3 - p1);
}

float Planef::distanceTo(const Vec3f &point) const {
	return std::fabsf(_normal.x * (point.x - _point.x) + _normal.y * (point.y - _point.y) + _normal.z * (point.z - _point.z)) / _normal.magnitude();
}

float Planef::signedDistanceTo(const Vec3f &point) const {
	return (_normal.x * (point.x - _point.x) + _normal.y * (point.y - _point.y) + _normal.z * (point.z - _point.z)) / _normal.magnitude();
}

void Planef::fromEquationCoeffs(float a, float b, float c, float d) {
	_normal.x = a;
	_normal.y = b;
	_normal.z = c;

	// For different possible orientations that produce infinity as a result
	if(a != 0.0f) {
		_point.x = -d / a;
		_point.y = 0.0f;
		_point.z = 0.0f;
	} else if(b != 0.0f) {
		_point.x = 0.0f;
		_point.y = -d / b;
		_point.z = 0.0f;
	} else {
		_point.x = 0.0f;
		_point.y = 0.0f;
		_point.z = -d / c;
	}
}