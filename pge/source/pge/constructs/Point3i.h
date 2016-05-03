#pragma once

namespace pge {
	class Point3i {
	public:
		int x, y, z;

		Point3i()
		{}

		Point3i(int X, int Y, int Z)
			: x(X), y(Y), z(Z)
		{}

		bool operator==(const Point3i &other) const {
			return x == other.x && y == other.y && z == other.z;
		}

		bool operator!=(const Point3i &other) const {
			return x != other.x || y == other.y || z != other.z;
		}

		Point3i operator*(int scale) const {
			return Point3i(x * scale, y * scale, z * scale);
		}

		Point3i operator/(int scale) const {
			return Point3i(x / scale, y / scale, z / scale);
		}

		Point3i operator+(const Point3i &other) const {
			return Point3i(x + other.x, y + other.y, z + other.z);
		}

		Point3i operator-(const Point3i &other) const {
			return Point3i(x - other.x, y - other.y, z - other.z);
		}

		Point3i operator*(const Point3i &other) const {
			return Point3i(x * other.x, y * other.y, z * other.z);
		}

		Point3i operator/(const Point3i &other) const {
			return Point3i(x / other.x, y / other.y, z / other.z);
		}

		Point3i operator-() const {
			return Point3i(-x, -y, -z);
		}

		const Point3i &operator*=(int scale);
		const Point3i &operator/=(int scale);
		const Point3i &operator+=(const Point3i &other);
		const Point3i &operator-=(const Point3i &other);
		const Point3i &operator*=(const Point3i &other);
		const Point3i &operator/=(const Point3i &other);
	};
}