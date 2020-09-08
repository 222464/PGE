#include "FormTriangle.h"

#include "../util/Functions.h"

#include <iostream>

using namespace pge;

FormTriangle::FormTriangle()
	: materialIndex(0)
{}

bool FormTriangle::rayTest(const Vec3f &start, const Vec3f &dir, float &t, Vec3f &hitPos) {
	Vec3f e1 = points[1] - points[0];
	Vec3f e2 = points[2] - points[0];

	Vec3f h = dir.cross(e2);
	float a = e1.dot(h);

	if(abs(a) < 0.00001f)
		return false;

	float f = 1.0f / a;

	Vec3f s = start - points[0];

	float u = f * s.dot(h);

	if(u < 0.0f || u > 1.0f)
		return false;

	Vec3f q = s.cross(e1);

	float v = f * dir.dot(q);

	if(v < 0.0f || u + v > 1.0f)
		return false;

	t = f * e2.dot(q);

	if(t > 0.00001f) {
		hitPos = start + dir * t;
		
		return true;
	}

	return false;
}

Vec3f FormTriangle::getBarycentricCoords(const Vec3f &hitPos) {
	// Find barycentric coordinates (area-weighted coordinates of hitPos)
	Vec3f f0 = points[0] - hitPos;
	Vec3f f1 = points[1] - hitPos;
	Vec3f f2 = points[2] - hitPos;

	Vec3f vecArea = (points[0] - points[1]).cross(points[0] - points[2]);
	Vec3f vecArea0 = f1.cross(f2);
	Vec3f vecArea1 = f2.cross(f0);
	Vec3f vecArea2 = f0.cross(f1);

	float areaInv = 1.0f / vecArea.magnitude();
	float area0 = vecArea0.magnitude() * areaInv * (vecArea.dot(vecArea0) > 0.0f ? 1.0f : -1.0f);
	float area1 = vecArea1.magnitude() * areaInv * (vecArea.dot(vecArea1) > 0.0f ? 1.0f : -1.0f);
	float area2 = vecArea2.magnitude() * areaInv * (vecArea.dot(vecArea2) > 0.0f ? 1.0f : -1.0f);

	return Vec3f(area0, area1, area2);
}

Vec2f FormTriangle::getImageCoord(const Vec3f &barycentricCoords) {
	return uv[0] * barycentricCoords.x + uv[1] * barycentricCoords.y + uv[2] * barycentricCoords.z;
}

Vec3f FormTriangle::getPosition(const Vec3f &barycentricCoords) {
	return points[0] * barycentricCoords.x + points[1] * barycentricCoords.y + points[2] * barycentricCoords.z;
}

Vec3f FormTriangle::getNormal(const Vec3f &barycentricCoords) {
	return normals[0] * barycentricCoords.x + normals[1] * barycentricCoords.y + normals[2] * barycentricCoords.z;
}

unsigned int FormTriangle::getMaterialIndex() {
	return materialIndex;
}

AABB3D FormTriangle::getAABB() {
	AABB3D aabb;

	aabb.lowerBound = aabb.upperBound = points[0];

	aabb.expand(points[1]);
	aabb.expand(points[2]);

	aabb.calculateHalfDims();
	aabb.calculateCenter();

	return aabb;
}

Vec3f FormTriangle::getCenter() {
	return (points[0] + points[1] + points[2]) * 0.33333f;
}

float FormTriangle::getArea() {
	float a = (points[0] - points[1]).magnitude();
	float b = (points[0] - points[2]).magnitude();
	float c = (points[1] - points[2]).magnitude();

	float s = (a + b + c) * 0.5f;

	return sqrtf(s * (s - a) * (s - b) * (s - c));
}

bool FormTriangle::isAdjacent(const FormTriangle &other) const {
	int numPointsEqual = 0;

	for (unsigned char i = 0; i < 3; i++)
	for (unsigned char j = 0; j < 3; j++)
	if (points[i] == other.points[j]) {
		numPointsEqual++;

		if (numPointsEqual == 2)
			return true;
	}

	return false;
}