#pragma once

#include <iostream>
#include <cmath>

namespace pge {
    class Vec2f  {
    public:
        float x, y;

        Vec2f()
        {}

        Vec2f(float X, float Y)
            : x(X), y(Y)
        {}

        bool operator==(const Vec2f &other) const {
            return x == other.x && y == other.y;
        }

        bool operator!=(const Vec2f &other) const {
            return x != other.x || y != other.y;
        }

        Vec2f operator*(float scale) const {
            return Vec2f(x * scale, y * scale);
        }

        Vec2f operator/(float scale) const {
            float scaleInv = 1.0f / scale;
            return Vec2f(x * scaleInv, y * scaleInv);
        }

        Vec2f operator+(const Vec2f &other) const {
            return Vec2f(x + other.x, y + other.y);
        }

        Vec2f operator-(const Vec2f &other) const {
            return Vec2f(x - other.x, y - other.y);
        }

        Vec2f operator*(const Vec2f &other) const {
            return Vec2f(x * other.x, y * other.y);
        }

        Vec2f operator/(const Vec2f &other) const {
            return Vec2f(x / other.x, y / other.y);
        }

        Vec2f operator-() const {
            return Vec2f(-x, -y);
        }

        const Vec2f &operator*=(float scale);
        const Vec2f &operator/=(float scale);
        const Vec2f &operator+=(const Vec2f &other);
        const Vec2f &operator-=(const Vec2f &other);
        const Vec2f &operator*=(const Vec2f &other);
        const Vec2f &operator/=(const Vec2f &other);

        float magnitude() const {
            return std::sqrt(x * x + y * y);
        }

        float magnitudeSquared() const {
            return x * x + y * y;
        }

        void normalize();
        Vec2f normalized() const;

        float dot(const Vec2f &other) const {
            return x * other.x + y * other.y;
        }

        Vec2f project(const Vec2f &other) const; // Projection of this on to other
    };

    static Vec2f lerp(const Vec2f &s, const Vec2f &e, float l) {
        return (e - s) * l + s;
    }

    static Vec2f operator*(float scale, const Vec2f &v) {
        return v * scale;
    }

    std::ostream &operator<<(std::ostream &output, const Vec2f &v);
}
