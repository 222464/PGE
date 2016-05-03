#pragma once

#include <pge/constructs/Vec3f.h>
#include <pge/constructs/Quaternion.h>

#include <LinearMath/btVector3.h>
#include <LinearMath/btQuaternion.h>

pge::Vec3f cons(const btVector3 &vec);
pge::Quaternion cons(const btQuaternion &quat);

btVector3 bt(const pge::Vec3f &vec);
btQuaternion bt(const pge::Quaternion &quat);