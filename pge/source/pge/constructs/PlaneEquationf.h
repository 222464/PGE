#pragma once

#include "Vec3f.h"

namespace pge {
	class PlaneEquationf {
	public:
		float a, b, c, d;

		PlaneEquationf()
		{}

		PlaneEquationf(float A, float B, float C, float D)
			: a(A), b(B), c(C), d(D)
		{}

		PlaneEquationf(const Vec3f &p1, const Vec3f &p2, const Vec3f &p3) {
			fromPoints(p1, p2, p3);
		}

		PlaneEquationf(const Vec3f &point, const Vec3f &normal) {
			fromAnchorNormal(point, normal);
		}

		void fromPoints(const Vec3f &p1, const Vec3f &p2, const Vec3f &p3);

		void fromAnchorNormal(const Vec3f &point, const Vec3f &normal);

		void normalizedFromEquationCoeffs(float A, float B, float C, float D);

		float distanceTo(const Vec3f &point) const;
		float signedDistanceTo(const Vec3f &point) const;
	};
}
