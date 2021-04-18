#include "BulletConversions.h"

pge::Vec3f cons(const btVector3 &vec) {
    return pge::Vec3f(vec.getX(), vec.getY(), vec.getZ());
}

pge::Quaternion cons(const btQuaternion &quat) {
    return pge::Quaternion(quat.getW(), quat.getX(), quat.getY(), quat.getZ());
}

btVector3 bt(const pge::Vec3f &vec) {
    return btVector3(vec.x, vec.y, vec.z);
}

btQuaternion bt(const pge::Quaternion &quat) {
    return btQuaternion(quat.x, quat.y, quat.z, quat.w);
}