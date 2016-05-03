#pragma once

#include <iostream>

namespace pge {
	class Vec4f  {
	public:
		float x, y, z, w;

		Vec4f()
		{}

		Vec4f(float X, float Y, float Z, float W)
			: x(X), y(Y), z(Z), w(W)
		{}

		bool operator==(const Vec4f &other) const {
			return x == other.x && y == other.y && z == other.z && w == other.w;
		}

		bool operator!=(const Vec4f &other) const {
			return x != other.x || y != other.y || z != other.z || w != other.w;
		}

		Vec4f operator*(float scale) const {
			return Vec4f(x * scale, y * scale, z * scale, w * scale);
		}

		Vec4f operator/(float scale) const {
			float scaleInv = 1.0f / scale;

			return Vec4f(x * scaleInv, y * scaleInv, z * scaleInv, w * scaleInv);
		}

		Vec4f operator+(const Vec4f &other) const {
			return Vec4f(x + other.x, y + other.y, z + other.z, w + other.w);
		}

		Vec4f operator-(const Vec4f &other) const {
			return Vec4f(x - other.x, y - other.y, z - other.z, w - other.w);
		}

		Vec4f operator*(const Vec4f &other) const {
			return Vec4f(x * other.x, y * other.y, z * other.z, w * other.w);
		}

		Vec4f operator/(const Vec4f &other) const {
			return Vec4f(x / other.x, y / other.y, z / other.z, w / other.w);
		}

		Vec4f operator-() const {
			return Vec4f(-x, -y, -z, -w);
		}

		const Vec4f &operator*=(float scale);
		const Vec4f &operator/=(float scale);
		const Vec4f &operator+=(const Vec4f &other);
		const Vec4f &operator-=(const Vec4f &other);
		const Vec4f &operator*=(const Vec4f &other);
		const Vec4f &operator/=(const Vec4f &other);

		float magnitude() const {
			return std::sqrtf(x * x + y * y + z * z + w * w);
		}

		float magnitudeSquared() const {
			return x * x + y * y + z * z + w * w;
		}

		void normalize();
		Vec4f normalized() const;

		float dot(const Vec4f &other) const {
			return x * other.x + y * other.y + z * other.z + w * other.w;
		}

		Vec4f project(const Vec4f &other) const; // Projection of this on to other
	};

	static Vec4f lerp(const Vec4f &s, const Vec4f &e, float l) {
		return (e - s) * l + s;
	}

	static Vec4f operator*(float scale, const Vec4f &v) {
		return v * scale;
	}

	std::ostream &operator<<(std::ostream &output, const Vec4f &v);
}