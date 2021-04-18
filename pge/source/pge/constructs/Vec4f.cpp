#include "Vec4f.h"

using namespace pge;

const Vec4f &Vec4f::operator*=(float scale) {
    x *= scale;
    y *= scale;
    z *= scale;
    w *= scale;

    return *this;
}

const Vec4f &Vec4f::operator/=(float scale) {
    float scaleInv = 1.0f / scale;

    x *= scaleInv;
    y *= scaleInv;
    z *= scaleInv;
    w *= scaleInv;

    return *this;
}

const Vec4f &Vec4f::operator+=(const Vec4f &other) {
    x += other.x;
    y += other.y;
    z += other.z;
    w += other.w;

    return *this;
}

const Vec4f &Vec4f::operator-=(const Vec4f &other) {
    x -= other.x;
    y -= other.y;
    z -= other.z;
    w -= other.w;

    return *this;
}

const Vec4f &Vec4f::operator*=(const Vec4f &other) {
    x *= other.x;
    y *= other.y;
    z *= other.z;
    w *= other.w;

    return *this;
}

const Vec4f &Vec4f::operator/=(const Vec4f &other) {
    x /= other.x;
    y /= other.y;
    z /= other.z;
    w /= other.w;

    return *this;
}

void Vec4f::normalize() {
    float mInv = 1.0f / magnitude();

    x *= mInv;
    y *= mInv;
    z *= mInv;
    w *= mInv;
}

Vec4f Vec4f::normalized() const {
    float mInv = magnitude();

    return Vec4f(x * mInv, y * mInv, z * mInv, w * mInv);
}

Vec4f Vec4f::project(const Vec4f &other) const {
    return (dot(other) / other.magnitudeSquared()) * other;
}

std::ostream &operator<<(std::ostream &output, const Vec4f &v) {
    std::cout << '(' << v.x << ", " << v.y << ", " << v.z << ')';
    return output;
}