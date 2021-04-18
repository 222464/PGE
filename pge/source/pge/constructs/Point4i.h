#pragma once

namespace pge {
    class Point4i {
    public:
        int x, y, z, w;

        Point4i()
        {}

        Point4i(int X, int Y, int Z, int W)
            : x(X), y(Y), z(Z), w(W)
        {}

        bool operator==(const Point4i &other) const {
            return x == other.x && y == other.y && z == other.z && w == other.w;
        }

        bool operator!=(const Point4i &other) const {
            return x != other.x || y == other.y || z != other.z || w != other.w;
        }

        Point4i operator*(int scale) const {
            return Point4i(x * scale, y * scale, z * scale, w * scale);
        }

        Point4i operator/(int scale) const {
            return Point4i(x / scale, y / scale, z / scale, w / scale);
        }

        Point4i operator+(const Point4i &other) const {
            return Point4i(x + other.x, y + other.y, z + other.z, w + other.w);
        }

        Point4i operator-(const Point4i &other) const {
            return Point4i(x - other.x, y - other.y, z - other.z, w - other.w);
        }

        Point4i operator*(const Point4i &other) const {
            return Point4i(x * other.x, y * other.y, z * other.z, w * other.w);
        }

        Point4i operator/(const Point4i &other) const {
            return Point4i(x / other.x, y / other.y, z / other.z, w / other.w);
        }

        Point4i operator-() const {
            return Point4i(-x, -y, -z, -w);
        }

        const Point4i &operator*=(int scale);
        const Point4i &operator/=(int scale);
        const Point4i &operator+=(const Point4i &other);
        const Point4i &operator-=(const Point4i &other);
        const Point4i &operator*=(const Point4i &other);
        const Point4i &operator/=(const Point4i &other);
    };
}