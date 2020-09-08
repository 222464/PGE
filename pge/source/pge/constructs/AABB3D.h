#pragma once

#include "Vec3f.h"

#include "Matrix4x4f.h"

namespace pge {
	class AABB3D {
	private:
		Vec3f center;
		Vec3f halfDims;

	public:
		Vec3f lowerBound;
		Vec3f upperBound;

		void calculateHalfDims() {
			halfDims = (upperBound - lowerBound) * 0.5f;
		}

		void calculateCenter() {
			center = lowerBound + halfDims;
		}

		void calculateBounds() {
			lowerBound = center - halfDims;
			upperBound = center + halfDims;
		}

		// Constructors
		AABB3D();
		AABB3D(const Vec3f &lowerBound, const Vec3f &upperBound);

		bool operator==(const AABB3D &other) {
			return lowerBound == other.lowerBound && upperBound == other.upperBound;
		}

		bool operator!=(const AABB3D &other) {
			return lowerBound != other.lowerBound || upperBound != other.upperBound;
		}

		// Accessors
		const Vec3f &getCenter() const {
			return center;
		}

		Vec3f getDims() const {
			return halfDims * 2.0f;
		}

		const Vec3f &getHalfDims() const {
			return halfDims;
		}

		const Vec3f &getLowerBound() const {
			return lowerBound;
		}

		const Vec3f &getUpperBound() const {
			return upperBound;
		}

		// Modifiers
		void setLowerBound(const Vec3f &lowerBound) {
			this->lowerBound = lowerBound;
		}

		void setUpperBound(const Vec3f &upperBound) {
			this->upperBound = upperBound;
		}

		void setCenter(const Vec3f &center);
		void incCenter(const Vec3f &increment);

		void setDims(const Vec3f &dims) {
			setHalfDims(dims * 0.5f);
		}

		void setHalfDims(const Vec3f &halfDims);

		void scale(const Vec3f &scale) {
			setHalfDims(halfDims * scale);
		}

		// Utility
		bool intersects(const AABB3D &other) const;
		bool intersects(const Vec3f &p1, const Vec3f &p2) const; // Line segment intersection test
		bool intersects(const Vec3f &start, const Vec3f &direction, float &t0, float &t1) const; // Line intersection test - changes t so point p = start + direction * t[...]. It is an array because it returns 2 points.
		bool contains(const AABB3D &other) const;
		bool contains(const Vec3f &vec) const;

		AABB3D getTransformedAABB(const Matrix4x4f &mat) const;

		Vec3f getVertexP(const Vec3f &normal) const;
		Vec3f getVertexN(const Vec3f &normal) const;

		// Maximum dimension
		float getRadius() const;

		void expand(const Vec3f &point);
		void expand(const AABB3D &other);
	};
}