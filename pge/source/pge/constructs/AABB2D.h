#pragma once

#include "Vec2f.h"
#include "Matrix4x4f.h"

namespace pge {
    class AABB2D
    {
    private:
        Vec2f center;
        Vec2f halfDims;

    public:
        Vec2f lowerBound;
        Vec2f upperBound;

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

        // Constructor
        AABB2D();
        AABB2D(const Vec2f &lowerBound, const Vec2f &upperBound);

        bool operator==(const AABB2D &other) {
            return lowerBound == other.lowerBound && upperBound == other.upperBound;
        }

        bool operator!=(const AABB2D &other) {
            return lowerBound != other.lowerBound || upperBound != other.upperBound;
        }

        // Accessors
        const Vec2f &getCenter() const {
            return center;
        }

        Vec2f getDims() const {
            return halfDims * 2.0f;
        }

        const Vec2f &getHalfDims() const {
            return halfDims;
        }

        const Vec2f &getLowerBound() const {
            return lowerBound;
        }

        const Vec2f &getUpperBound() const {
            return upperBound;
        }

        // Modifiers
        void setLowerBound(const Vec2f &lowerBound) {
            this->lowerBound = lowerBound;
        }

        void setUpperBound(const Vec2f &upperBound) {
            this->upperBound = upperBound;
        }

        void setCenter(const Vec2f &center);
        void incCenter(const Vec2f &increment);

        void setDims(const Vec2f &dims) {
            setHalfDims(dims * 0.5f);
        }

        void setHalfDims(const Vec2f &halfDims);

        void scale(const Vec2f &scale) {
            setHalfDims(halfDims * scale);
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