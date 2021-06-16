#include "Planef.h"

using namespace pge;

void Planef::fromPoints(const Vec3f &p1, const Vec3f &p2, const Vec3f &p3) {
    // Set anchor to any of the points
    point = p1;

    // Calculate the normal
    normal = (p2 - p1).cross(p3 - p1);
}

float Planef::distanceTo(const Vec3f &point) const {
    return std::abs(normal.x * (point.x - point.x) + normal.y * (point.y - point.y) + normal.z * (point.z - point.z)) / normal.magnitude();
}

float Planef::signedDistanceTo(const Vec3f &point) const {
    return (normal.x * (point.x - point.x) + normal.y * (point.y - point.y) + normal.z * (point.z - point.z)) / normal.magnitude();
}

void Planef::fromEquationCoeffs(float a, float b, float c, float d) {
    normal.x = a;
    normal.y = b;
    normal.z = c;

    // For different possible orientations that produce infinity as a result
    if(a != 0.0f) {
        point.x = -d / a;
        point.y = 0.0f;
        point.z = 0.0f;
    } else if(b != 0.0f) {
        point.x = 0.0f;
        point.y = -d / b;
        point.z = 0.0f;
    } else {
        point.x = 0.0f;
        point.y = 0.0f;
        point.z = -d / c;
    }
}
