#pragma once

#include <pge/constructs/Vec3f.h>

namespace pge {
	class Planef {
	public:
		Vec3f _point;
		Vec3f _normal;

		Planef()
		{}

		Planef(const Vec3f &point, const Vec3f &normal)
			: _point(point), _normal(normal)
		{}

		Planef(const Vec3f &p1, const Vec3f &p2, const Vec3f &p3) {
			fromPoints(p1, p2, p3);
		}

		Planef(float a, float b, float c, float d) {
			fromEquationCoeffs(a, b, c, d);
		}

		void fromPoints(const Vec3f &p1, const Vec3f &p2, const Vec3f &p3);

		void fromEquationCoeffs(float a, float b, float c, float d);

		float distanceTo(const Vec3f &point) const;
		float signedDistanceTo(const Vec3f &point) const;
	};
}