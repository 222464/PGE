#include <pge/constructs/Matrix3x3f.h>

#include <assert.h>

using namespace pge;

const float Matrix3x3f::_directionMatrixNormalizationTolerance = 0.9999f;

Matrix3x3f Matrix3x3f::operator*(const Matrix3x3f &other) const {
	Matrix3x3f product;

	for(int i = 0; i < 3; i++)
	for(int j = 0; j < 3; j++) {
		float sum = 0.0f;

		// jth row of this by ith column of other
		for(int d = 0; d < 3; d++)
			sum += get(d, j) * other.get(i, d);

		product.set(i, j, sum);
	}

	return product;
}

bool Matrix3x3f::operator==(const Matrix3x3f &other) const {
	for(int i = 0; i < 4; i++)
	for(int j = 0; j < 4; j++) {
		if(get(i, j) != other.get(i, j))
			return false;
	}

	return true;
}

void Matrix3x3f::setIdentity() {
	_elements[0] = 1.0f; _elements[3] = 0.0f; _elements[6] = 0.0f;
	_elements[1] = 0.0f; _elements[4] = 1.0f; _elements[7] = 0.0f;
	_elements[2] = 0.0f; _elements[5] = 0.0f; _elements[8] = 1.0f;
}

Matrix3x3f Matrix3x3f::transpose() const {
	Matrix3x3f transpose;

	for(int i = 0; i < 3; i++)
		for(int j = 0; j < 3; j++)
			transpose.set(j, i, get(i, j));

	return transpose;
}

float Matrix3x3f::determinant() const {
	return _elements[0] * _elements[4] * _elements[8] +
		_elements[3] * _elements[7] * _elements[2] +
		_elements[6] * _elements[1] * _elements[5] -
		_elements[2] * _elements[4] * _elements[6] - 
		_elements[5] * _elements[7] * _elements[0] -
		_elements[8] * _elements[1] * _elements[3];
}

bool Matrix3x3f::inverse(Matrix3x3f &inverse) const {
	float det = determinant();

	if(det == 0.0f)
		return false;

	float detInv = 1.0f / det;

	inverse._elements[0] = (_elements[4] * _elements[8] - _elements[5] * _elements[7]) * detInv;
	inverse._elements[1] = (_elements[7] * _elements[2] - _elements[8] * _elements[1]) * detInv;
	inverse._elements[2] = (_elements[1] * _elements[5] - _elements[2] * _elements[4]) * detInv;
	inverse._elements[3] = (_elements[6] * _elements[5] - _elements[8] * _elements[3]) * detInv;
	inverse._elements[4] = (_elements[0] * _elements[8] - _elements[2] * _elements[6]) * detInv;
	inverse._elements[5] = (_elements[3] * _elements[2] - _elements[5] * _elements[0]) * detInv;
	inverse._elements[6] = (_elements[3] * _elements[7] - _elements[4] * _elements[6]) * detInv;
	inverse._elements[7] = (_elements[6] * _elements[1] - _elements[7] * _elements[0]) * detInv;
	inverse._elements[8] = (_elements[0] * _elements[4] - _elements[1] * _elements[3]) * detInv;

	return true;
}

Matrix3x3f Matrix3x3f::scaleMatrix(const Vec2f &scale) {
	Matrix3x3f scaleMatrix;

	scaleMatrix.setIdentity();

	scaleMatrix.set(0, 0, scale.x);
	scaleMatrix.set(1, 1, scale.y);

	return scaleMatrix;
}

Matrix3x3f Matrix3x3f::translateMatrix(const Vec2f translation) {
	Matrix3x3f translateMatrix;

	translateMatrix.setIdentity();

	translateMatrix.set(2, 0, translation.x);
	translateMatrix.set(2, 1, translation.y);

	return translateMatrix;
}

Matrix3x3f Matrix3x3f::rotateMatrix(float angle) {
	float cosOfAngle = cosf(angle);
	float sinOfAngle = sinf(angle);

	Matrix3x3f rotationMatrix;

	rotationMatrix.setIdentity();

	rotationMatrix.set(0, 0, cosOfAngle);
	rotationMatrix.set(1, 0, sinOfAngle);
	rotationMatrix.set(0, 1, -sinOfAngle);
	rotationMatrix.set(1, 1, cosOfAngle);

	return rotationMatrix;
}

Matrix3x3f Matrix3x3f::identityMatrix() {
	Matrix3x3f identity;
	identity.setIdentity();

	return identity;
}

void Matrix3x3f::getUBOPadded(std::array<float, 12> &data) const {
	data[0] = _elements[0];
	data[1] = _elements[1];
	data[2] = _elements[2];
	data[3] = 0.0f;
	data[4] = _elements[3];
	data[5] = _elements[4];
	data[6] = _elements[5];
	data[7] = 0.0f;
	data[8] = _elements[6];
	data[9] = _elements[7];
	data[10] = _elements[8];
	data[11] = 0.0f;
}

Vec3f Matrix3x3f::operator*(const Vec3f &vec) const {
	Vec3f result;
	
	result.x = _elements[0] * vec.x + _elements[3] * vec.y + _elements[6] * vec.z;
	result.y = _elements[1] * vec.x + _elements[4] * vec.y + _elements[7] * vec.z;
	result.z = _elements[2] * vec.x + _elements[5] * vec.y + _elements[8] * vec.z;

	return result;
}

Vec2f Matrix3x3f::operator*(const Vec2f &vec) const {
	Vec2f result;

	result.x = _elements[0] * vec.x + _elements[3] * vec.y + _elements[6];
	result.y = _elements[1] * vec.x + _elements[4] * vec.y + _elements[7];

	return result;
}