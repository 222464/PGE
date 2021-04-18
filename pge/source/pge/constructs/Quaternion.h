#pragma once

#include "Vec3f.h"
#include "Vec4f.h"
#include "Matrix3x3f.h"
#include "Matrix4x4f.h"

namespace pge {
    class Quaternion {
    public:
        static const float quaternionNormalizationTolerance;
        static const float quaternionDotTolerance;

        float w, x, y, z;

        Quaternion()
        {}

        Quaternion(float W, float X, float Y, float Z)
            : w(W), x(X), y(Y), z(Z)
        {}

        Quaternion(const Vec3f &eulerAngles) {
            setFromEulerAngles(eulerAngles);
        }

        Quaternion(const Matrix3x3f &mat) {
            setFromRotationMatrix3x3f(mat);
        }

        Quaternion(float angle, const Vec3f &axis);

        float magnitude() const {
            return sqrtf(x * x + y * y + z * z + w * w);
        }

        float magnitudeSquared() const {
            return x * x + y * y + z * z + w * w;
        }

        Quaternion normalized() const;
        void normalize();

        float getAngle() const {
            return 2.0f * acosf(w);
        }

        Vec3f getAxis() const;

        Vec4f getVec4f() const {
            return Vec4f(x, y, z, w);
        }

        float dot(const Quaternion &other) const {
            return x * other.x + y * other.y + z * other.z + w * other.w;
        }

        Quaternion operator+(const Quaternion &other) const {
            return Quaternion(w + other.w, x + other.x, y + other.y, z + other.z);
        }

        Quaternion operator-(const Quaternion &other) const {
            return Quaternion(w - other.w, x - other.x, y - other.y, z - other.z);
        }

        Quaternion operator-() const {
            return Quaternion(-w, -x, -y, -z);
        }

        Quaternion operator*(float scale) const {
            return Quaternion(w * scale, x * scale, y * scale, z * scale);
        }

        Quaternion operator/(float scale) const {
            float scaleInv = 1.0f / scale;
            return Quaternion(w * scaleInv, x * scaleInv, y * scaleInv, z * scaleInv);
        }

        Quaternion operator*(const Quaternion &other) const;

        const Quaternion &operator*=(const Quaternion &other) {
            return *this = *this * other;
        }

        const Quaternion &operator*=(float scale) {
            return *this = *this * scale;
        }

        const Quaternion &operator/=(float scale) {
            return *this = *this / scale;
        }

        bool operator==(const Quaternion &other) const {
            return w == other.w && x == other.x && y == other.y && z == other.z;
        }

        bool operator!=(const Quaternion &other) const {
            return w != other.w || x != other.x || y != other.y || z != other.z;
        }

        Vec3f operator*(const Vec3f &vec);

        void rotate(float angle, const Vec3f &axis);
        void setFromRotateDifference(const Vec3f &v1, const Vec3f &v2);
        void setFromMatrix(const Matrix4x4f &mat);

        static Quaternion getRotated(float angle, const Vec3f &axis);
        static Quaternion getRotateDifference(const Vec3f &v1, const Vec3f &v2);
        static Quaternion getFromMatrix(const Matrix4x4f &mat);

        Quaternion conjugate() const {
            return Quaternion(w, -x, -y, -z);
        }

        Quaternion inverse() const {
            return conjugate() / magnitudeSquared();
        }

        void setIdentityMult();
        void setIdentityAdd();

        static Quaternion identityMult() {
            return Quaternion(1.0f, 0.0f, 0.0f, 0.0f);
        }

        static Quaternion identityAdd() {
            return Quaternion(0.0f, 0.0f, 0.0f, 0.0f);
        }

        Matrix4x4f getMatrix() const;

        void setFromEulerAngles(const Vec3f &eulerAngles);
        void setFromRotationMatrix3x3f(const Matrix3x3f &mat);
        Vec3f getEulerAngles() const;

        void calculateWFromXYZ();

        static Quaternion lerp(const Quaternion &first, const Quaternion &second, float interpolationCoefficient);
        static Quaternion slerp(const Quaternion &first, const Quaternion &second, float interpolationCoefficient);
    };

    Vec3f operator*(const Vec3f &vec, const Quaternion &quat);

    std::ostream &operator<<(std::ostream &output, const Quaternion &quat);
}