#include "Point2i.h"

using namespace pge;

const Point2i &Point2i::operator*=(int scale) {
	x *= scale;
	y *= scale;

	return *this;
}

const Point2i &Point2i::operator/=(int scale) {
	x /= scale;
	y /= scale;

	return *this;
}

const Point2i &Point2i::operator+=(const Point2i &other) {
	x += other.x;
	y += other.y;

	return *this;
}

const Point2i &Point2i::operator-=(const Point2i &other) {
	x -= other.x;
	y -= other.y;

	return *this;
}

const Point2i &Point2i::operator*=(const Point2i &other) {
	x *= other.x;
	y *= other.y;

	return *this;
}

const Point2i &Point2i::operator/=(const Point2i &other) {
	x /= other.x;
	y /= other.y;

	return *this;
}