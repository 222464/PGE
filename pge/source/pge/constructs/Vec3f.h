#pragma once

#include <iostream>
#include <cmath>

namespace pge {
    class Vec3f  {
    public:
        float x, y, z;

        Vec3f()
        {}

        Vec3f(float X, float Y, float Z)
            : x(X), y(Y), z(Z)
        {}

        bool operator==(const Vec3f &other) const {
            return x == other.x && y == other.y && z == other.z;
        }

        bool operator!=(const Vec3f &other) const {
            return x != other.x || y != other.y || z != other.z;
        }

        Vec3f operator*(float scale) const {
            return Vec3f(x * scale, y * scale, z * scale);
        }

        Vec3f operator/(float scale) const {
            float scaleInv = 1.0f / scale;
            return Vec3f(x * scaleInv, y * scaleInv, z * scaleInv);
        }

        Vec3f operator+(const Vec3f &other) const {
            return Vec3f(x + other.x, y + other.y, z + other.z);
        }

        Vec3f operator-(const Vec3f &other) const {
            return Vec3f(x - other.x, y - other.y, z - other.z);
        }

        Vec3f operator*(const Vec3f &other) const {
            return Vec3f(x * other.x, y * other.y, z * other.z);
        }

        Vec3f operator/(const Vec3f &other) const {
            return Vec3f(x / other.x, y / other.y, z / other.z);
        }

        Vec3f operator-() const {
            return Vec3f(-x, -y, -z);
        }

        const Vec3f &operator*=(float scale);
        const Vec3f &operator/=(float scale);
        const Vec3f &operator+=(const Vec3f &other);
        const Vec3f &operator-=(const Vec3f &other);
        const Vec3f &operator*=(const Vec3f &other);
        const Vec3f &operator/=(const Vec3f &other);

        float magnitude() const {
            return std::sqrt(x * x + y * y + z * z);
        }

        float magnitudeSquared() const {
            return x * x + y * y + z * z;
        }

        void normalize();
        Vec3f normalized() const;

        float dot(const Vec3f &other) const {
            return x * other.x + y * other.y + z * other.z;
        }

        Vec3f cross(const Vec3f &other) const;
        Vec3f project(const Vec3f &other) const; // Projection of this on to other
    };

    static Vec3f lerp(const Vec3f &s, const Vec3f &e, float l) {
        return (e - s) * l + s;
    }

    static Vec3f operator*(float scale, const Vec3f &v) {
        return v * scale;
    }

    std::ostream &operator<<(std::ostream &output, const Vec3f &v);
}
