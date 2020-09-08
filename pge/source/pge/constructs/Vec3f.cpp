#include "Vec3f.h"

using namespace pge;

const Vec3f &Vec3f::operator*=(float scale)  {
	x *= scale;
	y *= scale;
	z *= scale;

	return *this;
}

const Vec3f &Vec3f::operator/=(float scale)  {
	float scaleInv = 1.0f / scale;

	x *= scaleInv;
	y *= scaleInv;
	z *= scaleInv;

	return *this;
}

const Vec3f &Vec3f::operator+=(const Vec3f &other)  {
	x += other.x;
	y += other.y;
	z += other.z;

	return *this;
}

const Vec3f &Vec3f::operator-=(const Vec3f &other)  {
	x -= other.x;
	y -= other.y;
	z -= other.z;

	return *this;
}

const Vec3f &Vec3f::operator*=(const Vec3f &other)  {
	x *= other.x;
	y *= other.y;
	z *= other.z;

	return *this;
}

const Vec3f &Vec3f::operator/=(const Vec3f &other)  {
	x /= other.x;
	y /= other.y;
	z /= other.z;

	return *this;
}

void Vec3f::normalize()  {
	float mInv = 1.0f / magnitude();
	
	x *= mInv;
	y *= mInv;
	z *= mInv;
}

Vec3f Vec3f::normalized() const  {
	float mInv = 1.0f / magnitude();
	return Vec3f(x * mInv, y * mInv, z * mInv);
}

Vec3f Vec3f::cross(const Vec3f &other) const  {
	return Vec3f(y * other.z - z * other.y,
				 z * other.x - x * other.z,
				 x * other.y - y * other.x);
}

Vec3f Vec3f::project(const Vec3f &other) const {
	return (dot(other) / other.magnitudeSquared()) * other;
}

std::ostream &pge::operator<<(std::ostream &output, const Vec3f &v) {
	std::cout << '(' << v.x << ", " << v.y << ", " << v.z << ')';
	return output;
}