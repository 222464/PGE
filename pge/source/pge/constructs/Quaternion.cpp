#include "Quaternion.h"
#include "../util/Math.h"

using namespace pge;

const float Quaternion::quaternionNormalizationTolerance = 0.0001f;
const float Quaternion::quaternionDotTolerance = 0.9999f;

Quaternion::Quaternion(float angle, const Vec3f &axis) {
	angle *= 0.5f;

	float sinAngle = sinf(angle);
 
	w = cosf(angle);
	x = axis.x * sinAngle;
	y = axis.y * sinAngle;
	z = axis.z * sinAngle;
}

void Quaternion::normalize() {
	float m2 = x * x + y * y + z * z + w * w;

	if (fabsf(m2) > quaternionNormalizationTolerance && fabsf(m2 - 1.0f) > quaternionNormalizationTolerance) {
		float mInv = 1.0f / sqrtf(m2);

		w *= mInv;
		x *= mInv;
		y *= mInv;
		z *= mInv;
	}
}

Quaternion Quaternion::normalized() const {
	float m2 = x * x + y * y + z * z + w * w;

	if (fabsf(m2) > quaternionNormalizationTolerance && fabsf(m2 - 1.0f) > quaternionNormalizationTolerance) {
		float mInv = 1.0f / sqrtf(m2);

		return Quaternion(w * mInv, x * mInv, y * mInv, z * mInv);
	}

	return *this;
}

Vec3f Quaternion::getAxis() const {
	float s = sqrtf(1.0f - w * w);

	if (s < quaternionNormalizationTolerance)
		return Vec3f(x, y, z);

	float sInv = 1.0f / s;
	
	return Vec3f(x * sInv, y * sInv, z * sInv);
}

Quaternion Quaternion::operator*(const Quaternion &other) const {
	return Quaternion(w * other.w - x * other.x - y * other.y - z * other.z,
		w * other.x + x * other.w + y * other.z - z * other.y,
		w * other.y - x * other.z + y * other.w + z * other.x,
		w * other.z + x * other.y - y * other.x + z * other.w);
}

void Quaternion::rotate(float angle, const Vec3f &axis) {
	angle *= 0.5f;

	float sinAngle = sinf(angle);

	w = cosf(angle);
	x = axis.x * sinAngle;
	y = axis.y * sinAngle;
	z = axis.z * sinAngle;
}

void Quaternion::setFromRotateDifference(const Vec3f &v1, const Vec3f &v2) {
	Vec3f axis(v1.cross(v2));
	float angle = acosf(v1.dot(v2));

	rotate(angle, axis);
}

void Quaternion::setFromMatrix(const Matrix4x4f &mat) {
	float trace = mat.elements[0] + mat.elements[5] + mat.elements[10];

	if(trace > 0.0f) { 
		float s = sqrtf(trace + 1.0f) * 2.0f;

		float sInv = 1.0f / s;

		w = 0.25f * s;
		x = (mat.elements[9] - mat.elements[6]) * sInv;
		y = (mat.elements[2] - mat.elements[8]) * sInv; 
		z = (mat.elements[4] - mat.elements[1]) * sInv; 
	} else if(mat.elements[0] > mat.elements[5] && mat.elements[0] > mat.elements[10]) { 
		float s = sqrtf(1.0f + mat.elements[0] - mat.elements[5] - mat.elements[10]) * 2.0f;

		float sInv = 1.0f / s;

		w = (mat.elements[9] - mat.elements[6]) * sInv;
		x = 0.25f * s;
		y = (mat.elements[1] + mat.elements[4]) * sInv; 
		z = (mat.elements[2] + mat.elements[8]) * sInv; 
	} else if(mat.elements[5] > mat.elements[10]) { 
		float s = sqrtf(1.0f + mat.elements[5] - mat.elements[0] - mat.elements[10]) * 2.0f;

		float sInv = 1.0f / s;

		w = (mat.elements[2] - mat.elements[8]) * sInv;
		x = (mat.elements[1] + mat.elements[4]) * sInv; 
		y = 0.25f * s;
		z = (mat.elements[6] + mat.elements[9]) * sInv; 
	} else { 
		float s = sqrtf(1.0f + mat.elements[10] - mat.elements[0] - mat.elements[5]) * 2.0f;

		float sInv = 1.0f / s;

		w = (mat.elements[4] - mat.elements[1]) * sInv;
		x = (mat.elements[2] + mat.elements[8]) * sInv;
		y = (mat.elements[6] + mat.elements[9]) * sInv;
		z = 0.25f * s;
	}
}

Quaternion Quaternion::getRotated(float angle, const Vec3f &axis) {
	Quaternion quat;

	quat.rotate(angle, axis);

	return quat;
}

Quaternion Quaternion::getRotateDifference(const Vec3f &v1, const Vec3f &v2) {
	Quaternion quat;

	quat.setFromRotateDifference(v1, v2);

	return quat;
}

Quaternion Quaternion::getFromMatrix(const Matrix4x4f &mat) {
	Quaternion quat;

	quat.setFromMatrix(mat);

	return quat;
}

void Quaternion::setIdentityMult() {
	w = 1.0f;
	x = 0.0f;
	y = 0.0f;
	z = 0.0f;
}

void Quaternion::setIdentityAdd() {
	w = 0.0f;
	x = 0.0f;
	y = 0.0f;
	z = 0.0f;
}

Matrix4x4f Quaternion::getMatrix() const {
	Matrix4x4f mat;

	float x2 = x * x;
	float y2 = y * y;
	float z2 = z * z;
	float xy = x * y;
	float xz = x * z;
	float yz = y * z;
	float wx = w * x;
	float wy = w * y;
	float wz = w * z;

	mat.elements[0] = 1.0f - 2.0f * (y2 + z2);
	mat.elements[1] = 2.0f * (xy - wz);
	mat.elements[2] = 2.0f * (xz + wy);
	mat.elements[3] = 0.0f;
	mat.elements[4] = 2.0f * (xy + wz);
	mat.elements[5] = 1.0f - 2.0f * (x2 + z2);
	mat.elements[6] = 2.0f * (yz - wx);
	mat.elements[7] = 0.0f;
	mat.elements[8] = 2.0f * (xz - wy);
	mat.elements[9] = 2.0f * (yz + wx);
	mat.elements[10] = 1.0f - 2.0f * (x2 + y2);
	mat.elements[11] = 0.0f;
	mat.elements[12] = 0.0f;
	mat.elements[13] = 0.0f;
	mat.elements[14] = 0.0f;
	mat.elements[15] = 1.0f;

	return mat;
}

void Quaternion::setFromEulerAngles(const Vec3f &eulerAngles) {
	// Could also just multiply quaternions rotated along axes, can better optimize like this though
	float cx = cosf(eulerAngles.x / 2.0f);
	float cy = cosf(eulerAngles.y / 2.0f);
	float cz = cosf(eulerAngles.z / 2.0f);

	float sx = sinf(eulerAngles.x / 2.0f);
	float sy = sinf(eulerAngles.y / 2.0f);
	float sz = sinf(eulerAngles.z / 2.0f);

	w = cx * cy * cz - sx * sy * sz;
	x = cx * sy * sz + sx * cy * cz;
	y = cx * sy * cz + sx * cy * sz;
	z = cx * cy * sz - sx * sy * cz;
}

void Quaternion::setFromRotationMatrix3x3f(const Matrix3x3f &mat) {
	float trace = mat.elements[0] + mat.elements[4] + mat.elements[8];

	if(trace > 0.0f) {
		w = 0.5f * sqrtf(1.0f + trace);

		float wTimes4Inv = 1.0f / (w * 4.0f);

		x = (mat.elements[5] - mat.elements[7]) * wTimes4Inv;
		y = (mat.elements[6] - mat.elements[2]) * wTimes4Inv;
		z = (mat.elements[1] - mat.elements[3]) * wTimes4Inv;
	} else if(mat.elements[0] > mat.elements[4] && mat.elements[0] > mat.elements[8]) {
		float s = 2.0f * sqrtf(1.0f + mat.elements[0] - mat.elements[4] - mat.elements[8]);
		float sInv = 1.0f / s;

		w = (mat.elements[5] - mat.elements[7]) * sInv;
		x = 0.25f * s;
		y = (mat.elements[3] - mat.elements[1]) * sInv;
		z = (mat.elements[6] - mat.elements[2]) * sInv;
	} else if(mat.elements[4] > mat.elements[8]) {
		float s = 2.0f * sqrtf(1.0f + mat.elements[4] - mat.elements[0] - mat.elements[8]);
		float sInv = 1.0f / s;

		w = (mat.elements[6] - mat.elements[2]) * sInv;
		x = (mat.elements[3] - mat.elements[1]) * sInv;
		y = 0.25f * s;
		z = (mat.elements[7] - mat.elements[5]) * sInv;
	} else {
		float s = 2.0f * sqrtf(1.0f + mat.elements[8] - mat.elements[0] - mat.elements[4]);
		float sInv = 1.0f / s;

		w = (mat.elements[1] - mat.elements[3]) * sInv;
		x = (mat.elements[6] - mat.elements[2]) * sInv;
		y = (mat.elements[7] - mat.elements[5]) * sInv;
		z = 0.25f * s;
	}
}

Vec3f Quaternion::getEulerAngles() const {
	float x2 = x * x;
	float y2 = y * y;
	float z2 = z * z;
	float w2 = w * w;

	float unitLength = x2 + y2 + z2 + w2;

	float abcd = w * x + y * z;

	if(abcd > (0.5f - quaternionNormalizationTolerance) * unitLength)
		return Vec3f(2.0f * atan2f(y, w), pi, 0.0f);
	else if(abcd < (-0.5f + quaternionNormalizationTolerance) * unitLength)
		return Vec3f(-2.0f * atan2f(y, w), -pi, 0.0f);
	else {
		float adbc = w * z - x * y;
		float acbd = w * y - x * z;

		return Vec3f(atan2f(2.0f * adbc, 1.0f - 2.0f * (z2 + x2)), asinf(2.0f * abcd / unitLength), atan2f(2.0f * acbd, 1.0f - 2.0f * (y2 + x2)));
	}
}

void Quaternion::calculateWFromXYZ() {
	float t = 1.0f - x * x - y * y - z * z;

	if(t < 0.0f)
		w = 0.0f;
	else
		w = -sqrtf(t);
}

Quaternion Quaternion::lerp(const Quaternion &first, const Quaternion &second, float interpolationCoefficient) {
	Vec4f firstVec(first.getVec4f());
	Vec4f secondVec(second.getVec4f());
	Vec4f interp(firstVec + (secondVec - firstVec) * interpolationCoefficient);
	interp.normalize();

	return Quaternion(interp.w, interp.x, interp.y, interp.z);
}

Quaternion Quaternion::slerp(const Quaternion &first, const Quaternion &second, float interpolationCoefficient) {
	float dot = first.dot(second);

	if(dot > quaternionDotTolerance)
		return lerp(first, second, interpolationCoefficient);

	dot = clamp(dot, -1.0f, 1.0f);

	float theta = acosf(dot) * interpolationCoefficient;

	Quaternion third(second - first * dot);

	third.normalize();

	return first * cosf(theta) + third * sinf(theta);
}

Vec3f Quaternion::operator*(const Vec3f &vec) {
	Vec3f quatVec(x, y, z);
	Vec3f uv(quatVec.cross(vec));
	Vec3f uuv(quatVec.cross(uv));
	uv *= 2.0f * w;
	uuv *= 2.0f;

	return vec + uv + uuv;
}

Vec3f operator*(const Vec3f &vec, const Quaternion &quat) {
	return quat.inverse() * vec;
}

std::ostream &operator<<(std::ostream &output, const Quaternion &quat) {
	std::cout << "(" << quat.x << ", " << quat.y << ", " << quat.z << ", " << quat.w << ")";

	return output;
}