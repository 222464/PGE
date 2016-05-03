#include <pge/constructs/Point4i.h>

using namespace pge;

const Point4i &Point4i::operator*=(int scale) {
	x *= scale;
	y *= scale;
	z *= scale;
	w *= scale;

	return *this;
}

const Point4i &Point4i::operator/=(int scale) {
	x /= scale;
	y /= scale;
	z /= scale;
	w /= scale;

	return *this;
}

const Point4i &Point4i::operator+=(const Point4i &other) {
	x += other.x;
	y += other.y;
	z += other.z;
	w += other.w;

	return *this;
}

const Point4i &Point4i::operator-=(const Point4i &other) {
	x -= other.x;
	y -= other.y;
	z -= other.z;
	w -= other.w;

	return *this;
}

const Point4i &Point4i::operator*=(const Point4i &other) {
	x *= other.x;
	y *= other.y;
	z *= other.z;
	w *= other.w;

	return *this;
}

const Point4i &Point4i::operator/=(const Point4i &other) {
	x /= other.x;
	y /= other.y;
	z /= other.z;
	w /= other.w;

	return *this;
}