#include <pge/constructs/Matrix4x4f.h>

#include <pge/constructs/Matrix3x3f.h>

#include <assert.h>

using namespace pge;

const float Matrix4x4f::_directionMatrixNormalizationTolerance = 0.9999f;

Matrix4x4f Matrix4x4f::operator*(const Matrix4x4f &other) const {
	Matrix4x4f product;

	for (int i = 0; i < 4; i++)
	for (int j = 0; j < 4; j++) {
		float sum = 0.0f;

		// jth row of this by ith column of other
		for (int d = 0; d < 4; d++)
			sum += get(d, j) * other.get(i, d);

		product.set(i, j, sum);
	}

	return product;
}

bool Matrix4x4f::operator==(const Matrix4x4f &other) const {
	for (int i = 0; i < 4; i++)
	for (int j = 0; j < 4; j++) {
		if (get(i, j) != other.get(i, j))
			return false;
	}

	return true;
}

void Matrix4x4f::setIdentity() {
	_elements[0] = 1.0f; _elements[4] = 0.0f; _elements[8] = 0.0f; _elements[12] = 0.0f;
	_elements[1] = 0.0f; _elements[5] = 1.0f; _elements[9] = 0.0f; _elements[13] = 0.0f;
	_elements[2] = 0.0f; _elements[6] = 0.0f; _elements[10] = 1.0f; _elements[14] = 0.0f;
	_elements[3] = 0.0f; _elements[7] = 0.0f; _elements[11] = 0.0f; _elements[15] = 1.0f;
}

Matrix4x4f Matrix4x4f::transpose() const {
	Matrix4x4f transpose;

	for (int i = 0; i < 4; i++)
	for (int j = 0; j < 4; j++)
		transpose.set(j, i, get(i, j));

	return transpose;
}

bool Matrix4x4f::inverse(Matrix4x4f &inverse) const {
	inverse._elements[0] = _elements[5] * _elements[10] * _elements[15] -
		_elements[5] * _elements[11] * _elements[14] -
		_elements[9] * _elements[6] * _elements[15] +
		_elements[9] * _elements[7] * _elements[14] +
		_elements[13] * _elements[6] * _elements[11] -
		_elements[13] * _elements[7] * _elements[10];

	inverse._elements[4] = -_elements[4] * _elements[10] * _elements[15] +
		_elements[4] * _elements[11] * _elements[14] +
		_elements[8] * _elements[6] * _elements[15] -
		_elements[8] * _elements[7] * _elements[14] -
		_elements[12] * _elements[6] * _elements[11] +
		_elements[12] * _elements[7] * _elements[10];

	inverse._elements[8] = _elements[4] * _elements[9] * _elements[15] -
		_elements[4] * _elements[11] * _elements[13] -
		_elements[8] * _elements[5] * _elements[15] +
		_elements[8] * _elements[7] * _elements[13] +
		_elements[12] * _elements[5] * _elements[11] -
		_elements[12] * _elements[7] * _elements[9];

	inverse._elements[12] = -_elements[4] * _elements[9] * _elements[14] +
		_elements[4] * _elements[10] * _elements[13] +
		_elements[8] * _elements[5] * _elements[14] -
		_elements[8] * _elements[6] * _elements[13] -
		_elements[12] * _elements[5] * _elements[10] +
		_elements[12] * _elements[6] * _elements[9];

	inverse._elements[1] = -_elements[1] * _elements[10] * _elements[15] +
		_elements[1] * _elements[11] * _elements[14] +
		_elements[9] * _elements[2] * _elements[15] -
		_elements[9] * _elements[3] * _elements[14] -
		_elements[13] * _elements[2] * _elements[11] +
		_elements[13] * _elements[3] * _elements[10];

	inverse._elements[5] = _elements[0] * _elements[10] * _elements[15] -
		_elements[0] * _elements[11] * _elements[14] -
		_elements[8] * _elements[2] * _elements[15] +
		_elements[8] * _elements[3] * _elements[14] +
		_elements[12] * _elements[2] * _elements[11] -
		_elements[12] * _elements[3] * _elements[10];

	inverse._elements[9] = -_elements[0] * _elements[9] * _elements[15] +
		_elements[0] * _elements[11] * _elements[13] +
		_elements[8] * _elements[1] * _elements[15] -
		_elements[8] * _elements[3] * _elements[13] -
		_elements[12] * _elements[1] * _elements[11] +
		_elements[12] * _elements[3] * _elements[9];

	inverse._elements[13] = _elements[0] * _elements[9] * _elements[14] -
		_elements[0] * _elements[10] * _elements[13] -
		_elements[8] * _elements[1] * _elements[14] +
		_elements[8] * _elements[2] * _elements[13] +
		_elements[12] * _elements[1] * _elements[10] -
		_elements[12] * _elements[2] * _elements[9];

	inverse._elements[2] = _elements[1] * _elements[6] * _elements[15] -
		_elements[1] * _elements[7] * _elements[14] -
		_elements[5] * _elements[2] * _elements[15] +
		_elements[5] * _elements[3] * _elements[14] +
		_elements[13] * _elements[2] * _elements[7] -
		_elements[13] * _elements[3] * _elements[6];

	inverse._elements[6] = -_elements[0] * _elements[6] * _elements[15] +
		_elements[0] * _elements[7] * _elements[14] +
		_elements[4] * _elements[2] * _elements[15] -
		_elements[4] * _elements[3] * _elements[14] -
		_elements[12] * _elements[2] * _elements[7] +
		_elements[12] * _elements[3] * _elements[6];

	inverse._elements[10] = _elements[0] * _elements[5] * _elements[15] -
		_elements[0] * _elements[7] * _elements[13] -
		_elements[4] * _elements[1] * _elements[15] +
		_elements[4] * _elements[3] * _elements[13] +
		_elements[12] * _elements[1] * _elements[7] -
		_elements[12] * _elements[3] * _elements[5];

	inverse._elements[14] = -_elements[0] * _elements[5] * _elements[14] +
		_elements[0] * _elements[6] * _elements[13] +
		_elements[4] * _elements[1] * _elements[14] -
		_elements[4] * _elements[2] * _elements[13] -
		_elements[12] * _elements[1] * _elements[6] +
		_elements[12] * _elements[2] * _elements[5];

	inverse._elements[3] = -_elements[1] * _elements[6] * _elements[11] +
		_elements[1] * _elements[7] * _elements[10] +
		_elements[5] * _elements[2] * _elements[11] -
		_elements[5] * _elements[3] * _elements[10] -
		_elements[9] * _elements[2] * _elements[7] +
		_elements[9] * _elements[3] * _elements[6];

	inverse._elements[7] = _elements[0] * _elements[6] * _elements[11] -
		_elements[0] * _elements[7] * _elements[10] -
		_elements[4] * _elements[2] * _elements[11] +
		_elements[4] * _elements[3] * _elements[10] +
		_elements[8] * _elements[2] * _elements[7] -
		_elements[8] * _elements[3] * _elements[6];

	inverse._elements[11] = -_elements[0] * _elements[5] * _elements[11] +
		_elements[0] * _elements[7] * _elements[9] +
		_elements[4] * _elements[1] * _elements[11] -
		_elements[4] * _elements[3] * _elements[9] -
		_elements[8] * _elements[1] * _elements[7] +
		_elements[8] * _elements[3] * _elements[5];

	inverse._elements[15] = _elements[0] * _elements[5] * _elements[10] -
		_elements[0] * _elements[6] * _elements[9] -
		_elements[4] * _elements[1] * _elements[10] +
		_elements[4] * _elements[2] * _elements[9] +
		_elements[8] * _elements[1] * _elements[6] -
		_elements[8] * _elements[2] * _elements[5];

	// Easier to calculate determinant using inverse matrix so far
	float det = _elements[0] * inverse._elements[0] +
		_elements[1] * inverse._elements[4] +
		_elements[2] * inverse._elements[8] +
		_elements[3] * inverse._elements[12];

	if (det == 0.0f)
		return false;

	det = 1.0f / det;

	for (int i = 0; i < 16; i++)
		inverse._elements[i] *= det;

	return true;
}

Matrix3x3f Matrix4x4f::getUpperLeftMatrix3x3f() {
	Matrix3x3f mat;

	mat._elements[0] = _elements[0];
	mat._elements[1] = _elements[1];
	mat._elements[2] = _elements[2];
	mat._elements[3] = _elements[4];
	mat._elements[4] = _elements[5];
	mat._elements[5] = _elements[6];
	mat._elements[6] = _elements[8];
	mat._elements[7] = _elements[9];
	mat._elements[8] = _elements[10];

	return mat;
}

Matrix4x4f Matrix4x4f::scaleMatrix(const Vec3f &scale) {
	Matrix4x4f scaleMatrix;

	scaleMatrix.setIdentity();

	scaleMatrix.set(0, 0, scale.x);
	scaleMatrix.set(1, 1, scale.y);
	scaleMatrix.set(2, 2, scale.z);

	return scaleMatrix;
}

Matrix4x4f Matrix4x4f::translateMatrix(const Vec3f translation) {
	Matrix4x4f translateMatrix;

	translateMatrix.setIdentity();

	translateMatrix.set(3, 0, translation.x);
	translateMatrix.set(3, 1, translation.y);
	translateMatrix.set(3, 2, translation.z);

	return translateMatrix;
}

Matrix4x4f Matrix4x4f::rotateMatrixX(float angle) {
	float cosOfAngle = std::cosf(angle);
	float sinOfAngle = std::sinf(angle);

	Matrix4x4f rotationMatrix;

	rotationMatrix.setIdentity();

	rotationMatrix.set(1, 1, cosOfAngle);
	rotationMatrix.set(2, 1, sinOfAngle);
	rotationMatrix.set(1, 2, -sinOfAngle);
	rotationMatrix.set(2, 2, cosOfAngle);

	return rotationMatrix;
}

Matrix4x4f Matrix4x4f::rotateMatrixY(float angle) {
	float cosOfAngle = std::cosf(angle);
	float sinOfAngle = std::sinf(angle);

	Matrix4x4f rotationMatrix;

	rotationMatrix.setIdentity();

	rotationMatrix.set(0, 0, cosOfAngle);
	rotationMatrix.set(2, 0, -sinOfAngle);
	rotationMatrix.set(0, 2, sinOfAngle);
	rotationMatrix.set(2, 2, cosOfAngle);

	return rotationMatrix;
}

Matrix4x4f Matrix4x4f::rotateMatrixZ(float angle) {
	float cosOfAngle = std::cosf(angle);
	float sinOfAngle = std::sinf(angle);

	Matrix4x4f rotationMatrix;

	rotationMatrix.setIdentity();

	rotationMatrix.set(0, 0, cosOfAngle);
	rotationMatrix.set(1, 0, sinOfAngle);
	rotationMatrix.set(0, 1, -sinOfAngle);
	rotationMatrix.set(1, 1, cosOfAngle);

	return rotationMatrix;
}

Matrix4x4f Matrix4x4f::directionMatrix(const Vec3f &direction, const Vec3f &up) {
	Vec3f tangent_0 = direction.cross(up);

	tangent_0.normalize();

	Vec3f tangent_1(direction.cross(tangent_0).normalized());

	Matrix4x4f mat;

	mat._elements[0] = direction.x;
	mat._elements[1] = direction.y;
	mat._elements[2] = direction.z;
	mat._elements[3] = 0.0f;

	mat._elements[4] = tangent_1.x;
	mat._elements[5] = tangent_1.y;
	mat._elements[6] = tangent_1.z;
	mat._elements[7] = 0.0f;

	mat._elements[8] = tangent_0.x;
	mat._elements[9] = tangent_0.y;
	mat._elements[10] = tangent_0.z;
	mat._elements[11] = 0.0f;

	mat._elements[12] = 0.0f;
	mat._elements[13] = 0.0f;
	mat._elements[14] = 0.0f;
	mat._elements[15] = 1.0f;

	return mat;
}

Matrix4x4f Matrix4x4f::directionMatrixAutoUp(const Vec3f &direction) {
	Vec3f tangent_0 = direction.cross(Vec3f(1.0f, 0.0f, 0.0f)); // Try x as up vector

	if (tangent_0.magnitudeSquared() < 0.00001f) // x up vector didn't work (the direction is too close along x axis, cannot cross parallel vecs), try another
		tangent_0 = direction.cross(Vec3f(0.0f, 1.0f, 0.0f));

	tangent_0.normalize();

	Vec3f tangent_1(direction.cross(tangent_0).normalized());

	Matrix4x4f mat;

	mat._elements[0] = direction.x;
	mat._elements[1] = direction.y;
	mat._elements[2] = direction.z;
	mat._elements[3] = 0.0f;

	mat._elements[4] = tangent_0.x;
	mat._elements[5] = tangent_0.y;
	mat._elements[6] = tangent_0.z;
	mat._elements[7] = 0.0f;

	mat._elements[8] = tangent_1.x;
	mat._elements[9] = tangent_1.y;
	mat._elements[10] = tangent_1.z;
	mat._elements[11] = 0.0f;

	mat._elements[12] = 0.0f;
	mat._elements[13] = 0.0f;
	mat._elements[14] = 0.0f;
	mat._elements[15] = 1.0f;

	return mat;
}

Matrix4x4f Matrix4x4f::cameraDirectionMatrix(const Vec3f &direction, const Vec3f &up) {
	Vec3f tangent_0 = direction.cross(up);

	tangent_0.normalize();

	Vec3f tangent_1(tangent_0.cross(direction).normalized());

	Matrix4x4f mat;

	mat._elements[0] = tangent_0.x;
	mat._elements[4] = tangent_0.y;
	mat._elements[8] = tangent_0.z;
	mat._elements[12] = 0.0f;

	mat._elements[1] = tangent_1.x;
	mat._elements[5] = tangent_1.y;
	mat._elements[9] = tangent_1.z;
	mat._elements[13] = 0.0f;

	mat._elements[2] = -direction.x;
	mat._elements[6] = -direction.y;
	mat._elements[10] = -direction.z;
	mat._elements[14] = 0.0f;

	mat._elements[3] = 0.0f;
	mat._elements[7] = 0.0f;
	mat._elements[11] = 0.0f;
	mat._elements[15] = 1.0f;

	return mat;
}

Matrix4x4f Matrix4x4f::cameraDirectionMatrixAutoUp(const Vec3f &direction) {
	Vec3f tangent_0 = direction.cross(Vec3f(1.0f, 0.0f, 0.0f)); // Try x as up vector

	if (tangent_0.magnitudeSquared() < 0.001f) // x up vector didn't work (the direction is too close along x axis, cannot cross parallel vecs), try another
		tangent_0 = direction.cross(Vec3f(0.0f, 1.0f, 0.0f));

	tangent_0.normalize();

	Vec3f tangent_1(tangent_0.cross(direction).normalized());

	Matrix4x4f mat;

	mat._elements[0] = tangent_0.x;
	mat._elements[4] = tangent_0.y;
	mat._elements[8] = tangent_0.z;
	mat._elements[12] = 0.0f;

	mat._elements[1] = tangent_1.x;
	mat._elements[5] = tangent_1.y;
	mat._elements[9] = tangent_1.z;
	mat._elements[13] = 0.0f;

	mat._elements[2] = -direction.x;
	mat._elements[6] = -direction.y;
	mat._elements[10] = -direction.z;
	mat._elements[14] = 0.0f;

	mat._elements[3] = 0.0f;
	mat._elements[7] = 0.0f;
	mat._elements[11] = 0.0f;
	mat._elements[15] = 1.0f;

	return mat;
}

Matrix4x4f Matrix4x4f::identityMatrix() {
	Matrix4x4f identity;
	identity.setIdentity();

	return identity;
}

Matrix4x4f Matrix4x4f::perspectiveMatrix(float fovY, float aspectRatio, float zNear, float zFar) {
	float f = 1.0f / std::tanf(0.5f * fovY);

	float nearMinusFar = zNear - zFar;

	Matrix4x4f mat;

	mat._elements[0] = f / aspectRatio;
	mat._elements[4] = 0.0f;
	mat._elements[8] = 0.0f;
	mat._elements[12] = 0.0f;

	mat._elements[1] = 0.0f;
	mat._elements[5] = f;
	mat._elements[9] = 0.0f;
	mat._elements[13] = 0.0f;

	mat._elements[2] = 0.0f;
	mat._elements[6] = 0.0f;
	mat._elements[10] = (zNear + zFar) / nearMinusFar;
	mat._elements[14] = (2.0f * zNear * zFar) / nearMinusFar;

	mat._elements[3] = 0.0f;
	mat._elements[7] = 0.0f;
	mat._elements[11] = -1.0f;
	mat._elements[15] = 0.0f;

	return mat;
}

Matrix4x4f Matrix4x4f::orthoMatrix(float left, float right, float bottom, float top, float zNear, float zFar) {
	float rightMinusLeftInv = 1.0f / (right - left);
	float topMinusBottomInv = 1.0f / (top - bottom);
	float farMinusNearInv = 1.0f / (zFar - zNear);

	Matrix4x4f mat;

	mat._elements[0] = 2.0f * rightMinusLeftInv;
	mat._elements[1] = 0.0f;
	mat._elements[2] = 0.0f;
	mat._elements[3] = 0.0f;
	mat._elements[4] = 0.0f;
	mat._elements[5] = 2.0f * topMinusBottomInv;
	mat._elements[6] = 0.0f;
	mat._elements[7] = 0.0f;
	mat._elements[8] = 0.0f;
	mat._elements[9] = 0.0f;
	mat._elements[10] = -2.0f * farMinusNearInv;
	mat._elements[11] = 0.0f;
	mat._elements[12] = -(right + left) * rightMinusLeftInv;
	mat._elements[13] = -(top + bottom) * topMinusBottomInv;
	mat._elements[14] = -(zFar + zNear) * farMinusNearInv;
	mat._elements[15] = 1.0f;

	return mat;
}

Vec4f Matrix4x4f::operator*(const Vec4f &vec) const {
	Vec4f result;

	result.x = _elements[0] * vec.x + _elements[4] * vec.y + _elements[8] * vec.z + _elements[12];
	result.y = _elements[1] * vec.x + _elements[5] * vec.y + _elements[9] * vec.z + _elements[13];
	result.z = _elements[2] * vec.x + _elements[6] * vec.y + _elements[10] * vec.z + _elements[14];
	result.w = _elements[3] * vec.x + _elements[7] * vec.y + _elements[11] * vec.z + _elements[15];

	return result;
}

Vec3f Matrix4x4f::operator*(const Vec3f &vec) const {
	Vec3f result;

	result.x = _elements[0] * vec.x + _elements[4] * vec.y + _elements[8] * vec.z + _elements[12];
	result.y = _elements[1] * vec.x + _elements[5] * vec.y + _elements[9] * vec.z + _elements[13];
	result.z = _elements[2] * vec.x + _elements[6] * vec.y + _elements[10] * vec.z + _elements[14];

	return result;
}