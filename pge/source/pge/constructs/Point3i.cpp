#include "Point3i.h"

using namespace pge;

const Point3i &Point3i::operator*=(int scale) {
	x *= scale;
	y *= scale;
	z *= scale;

	return *this;
}

const Point3i &Point3i::operator/=(int scale) {
	x /= scale;
	y /= scale;
	z /= scale;

	return *this;
}

const Point3i &Point3i::operator+=(const Point3i &other) {
	x += other.x;
	y += other.y;
	z += other.z;

	return *this;
}

const Point3i &Point3i::operator-=(const Point3i &other) {
	x -= other.x;
	y -= other.y;
	z -= other.z;

	return *this;
}

const Point3i &Point3i::operator*=(const Point3i &other) {
	x *= other.x;
	y *= other.y;
	z *= other.z;

	return *this;
}

const Point3i &Point3i::operator/=(const Point3i &other) {
	x /= other.x;
	y /= other.y;
	z /= other.z;

	return *this;
}