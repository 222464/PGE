#include "Vec2f.h"

using namespace pge;

const Vec2f &Vec2f::operator*=(float scale)  {
    x *= scale;
    y *= scale;

    return *this;
}

const Vec2f &Vec2f::operator/=(float scale)  {
    float scaleInv = 1.0f / scale;

    x *= scaleInv;
    y *= scaleInv;

    return *this;
}

const Vec2f &Vec2f::operator+=(const Vec2f &other)  {
    x += other.x;
    y += other.y;

    return *this;
}

const Vec2f &Vec2f::operator-=(const Vec2f &other)  {
    x -= other.x;
    y -= other.y;
    
    return *this;
}

const Vec2f &Vec2f::operator*=(const Vec2f &other)  {
    x *= other.x;
    y *= other.y;

    return *this;
}

const Vec2f &Vec2f::operator/=(const Vec2f &other)  {
    x /= other.x;
    y /= other.y;

    return *this;
}

void Vec2f::normalize()  {
    float mInv = 1.0f / magnitude();

    x *= mInv;
    y *= mInv;
}

Vec2f Vec2f::normalized() const  {
    float mInv = 1.0f / magnitude();

    return Vec2f(x * mInv, y * mInv);
}

Vec2f Vec2f::project(const Vec2f &other) const {
    return (dot(other) / other.magnitudeSquared()) * other;
}

std::ostream &operator<<(std::ostream &output, const Vec2f &v) {
    std::cout << '(' << v.x << ", " << v.y << ')';
    return output;
}