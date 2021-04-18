#pragma once

namespace pge {
    class Point2i {
    public:
        int x, y;

        Point2i()
        {}

        Point2i(int X, int Y)
            : x(X), y(Y)
        {}

        bool operator==(const Point2i &other) const {
            return x == other.x && y == other.y;
        }

        bool operator!=(const Point2i &other) const {
            return x != other.x || y != other.y;
        }

        Point2i operator*(int scale) const {
            return Point2i(x * scale, y * scale);
        }

        Point2i operator/(int scale) const {
            return Point2i(x / scale, y / scale);
        }

        Point2i operator+(const Point2i &other) const {
            return Point2i(x + other.x, y + other.y);
        }

        Point2i operator-(const Point2i &other) const {
            return Point2i(x - other.x, y - other.y);
        }

        Point2i operator*(const Point2i &other) const {
            return Point2i(x * other.x, y * other.y);
        }

        Point2i operator/(const Point2i &other) const {
            return Point2i(x / other.x, y / other.y);
        }

        Point2i operator-() const {
            return Point2i(-x, -y);
        }

        const Point2i &operator*=(int scale);
        const Point2i &operator/=(int scale);
        const Point2i &operator+=(const Point2i &other);
        const Point2i &operator-=(const Point2i &other);
        const Point2i &operator*=(const Point2i &other);
        const Point2i &operator/=(const Point2i &other);
    };
}