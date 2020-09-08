#pragma once

#include "Form.h"

#include "../constructs/Vec2f.h"
#include "../constructs/Vec3f.h"

#include "../constructs/AABB3D.h"

#include <array>

namespace pge {
	class FormTriangle : public Form {
	public:
		unsigned short materialIndex;

		std::array<Vec3f, 3> points;
		std::array<Vec3f, 3> normals;
		std::array<Vec2f, 3> uv;

		FormTriangle();

		// Inherited from Form
		bool rayTest(const Vec3f &start, const Vec3f &dir, float &t, Vec3f &hitPos);
		Vec3f getBarycentricCoords(const Vec3f &hitPos);
		Vec2f getImageCoord(const Vec3f &barycentricCoords);
		Vec3f getPosition(const Vec3f &barycentricCoords);
		Vec3f getNormal(const Vec3f &barycentricCoords);
		unsigned int getMaterialIndex();
		AABB3D getAABB();
		Vec3f getCenter();
		float getArea();

		bool isAdjacent(const FormTriangle &other) const;
	};
}
