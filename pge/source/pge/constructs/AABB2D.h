#pragma once

#include <pge/constructs/Vec2f.h>
#include <pge/constructs/Matrix4x4f.h>

namespace pge {
	class AABB2D
	{
	private:
		Vec2f _center;
		Vec2f _halfDims;

	public:
		Vec2f _lowerBound;
		Vec2f _upperBound;

		void calculateHalfDims() {
			_halfDims = (_upperBound - _lowerBound) * 0.5f;
		}

		void calculateCenter() {
			_center = _lowerBound + _halfDims;
		}

		void calculateBounds() {
			_lowerBound = _center - _halfDims;
			_upperBound = _center + _halfDims;
		}

		// Constructor
		AABB2D();
		AABB2D(const Vec2f &lowerBound, const Vec2f &upperBound);

		bool operator==(const AABB2D &other) {
			return _lowerBound == other._lowerBound && _upperBound == other._upperBound;
		}

		bool operator!=(const AABB2D &other) {
			return _lowerBound != other._lowerBound || _upperBound != other._upperBound;
		}

		// Accessors
		const Vec2f &getCenter() const {
			return _center;
		}

		Vec2f getDims() const {
			return _halfDims * 2.0f;
		}

		const Vec2f &getHalfDims() const {
			return _halfDims;
		}

		const Vec2f &getLowerBound() const {
			return _lowerBound;
		}

		const Vec2f &getUpperBound() const {
			return _upperBound;
		}

		// Modifiers
		void setLowerBound(const Vec2f &lowerBound) {
			_lowerBound = lowerBound;
		}

		void setUpperBound(const Vec2f &upperBound) {
			_upperBound = upperBound;
		}

		void setCenter(const Vec2f &center);
		void incCenter(const Vec2f &increment);

		void setDims(const Vec2f &dims) {
			setHalfDims(dims * 0.5f);
		}

		void setHalfDims(const Vec2f &halfDims);

		void scale(const Vec2f &scale) {
			setHalfDims(_halfDims * scale);
		}

		// Utility
		bool intersects(const AABB2D &other) const;
		bool contains(const AABB2D &other) const;
		bool contains(const Vec2f &vec) const;

		AABB2D getTransformedAABB2D(const Matrix4x4f &mat) const;

		// Maximum dimension
		float getRadius() const;
	};
}