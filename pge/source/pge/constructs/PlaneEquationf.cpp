#include <pge/constructs/PlaneEquationf.h>

using namespace pge;

void PlaneEquationf::fromPoints(const Vec3f &p1, const Vec3f &p2, const Vec3f &p3) {
	// Set point to any of the points
	Vec3f point = p1;

	// Calculate the normal
	Vec3f normal = (p2 - p1).cross(p3 - p1);

	a = normal.x;
	b = normal.y;
	c = normal.z;

	d = -(a * point.x + b * point.y + c * point.z);
}

void PlaneEquationf::fromAnchorNormal(const Vec3f &point, const Vec3f &normal) {
	a = normal.x;
	b = normal.y;
	c = normal.z;

	d = -(a * point.x + b * point.y + c * point.z);
}

void PlaneEquationf::normalizedFromEquationCoeffs(float A, float B, float C, float D) {
	float magInv = 1.0f / sqrtf(A * A + B * B + C * C);

	a = A * magInv;
	b = B * magInv;
	c = C * magInv;
	d = D * magInv;
}

float PlaneEquationf::distanceTo(const Vec3f &point) const {
	return fabsf(a * point.x + b * point.y + c * point.z + d) / sqrtf(a * a + b * b + c * c);
}

float PlaneEquationf::signedDistanceTo(const Vec3f &point) const {
	return (a * point.x + b * point.y + c * point.z + d) / sqrtf(a * a + b * b + c * c);
}