#pragma once

#include "Vec3f.h"
#include "Vec4f.h"
#include <vector>
#include <array>

/*
	Different from mathematics standard:

	i means column
	j means row
	*/

// 1D array stores the matrix in column-major order, like OpenGL
namespace pge {
	class Matrix4x4f {
	public:
		static const float directionMatrixNormalizationTolerance;

		std::array<float, 16> elements;

		Matrix4x4f()
		{}

		Matrix4x4f(std::vector<float> sourceArray) {
			for (size_t i = 0; i < 16; i++)
				elements[i] = sourceArray[i];
		}

		Matrix4x4f(std::array<float, 16> sourceArray) {
			elements = sourceArray;
		}

		float &operator[](int i) {
			return elements[i];
		}

		Matrix4x4f operator*(const Matrix4x4f &other) const;

		Matrix4x4f operator*=(const Matrix4x4f &other) {
			return (*this) = (*this) * other;
		}

		bool operator==(const Matrix4x4f &other) const;

		bool operator!=(const Matrix4x4f &other) const {
			return !((*this) == other);
		}

		void set(int i, int j, float val) {
			elements[4 * i + j] = val; // Row-major would be i + 4 * j
		}

		float get(int i, int j) const {
			return elements[4 * i + j]; // Row-major would be i + 4 * j
		}

		void setIdentity();
		Matrix4x4f transpose() const;

		// Returns false if there is no inverse, true if there is one
		// Warning! Expensive function!
		bool inverse(Matrix4x4f &inverse) const;

		class Matrix3x3f getUpperLeftMatrix3x3f();

		// Transformation matrix generators
		static Matrix4x4f scaleMatrix(const Vec3f &scale);
		static Matrix4x4f translateMatrix(const Vec3f translation);
		static Matrix4x4f rotateMatrixX(float angle);
		static Matrix4x4f rotateMatrixY(float angle);
		static Matrix4x4f rotateMatrixZ(float angle);

		static Matrix4x4f rotateMatrix(const Vec3f &eulerAngles) {
			return rotateMatrixX(eulerAngles.x) * rotateMatrixY(eulerAngles.y) * rotateMatrixZ(eulerAngles.z);
		}

		static Matrix4x4f directionMatrix(const Vec3f &direction, const Vec3f &up);
		static Matrix4x4f directionMatrixAutoUp(const Vec3f &direction); // Chooses up such that it doesn't lock up
		static Matrix4x4f cameraDirectionMatrix(const Vec3f &direction, const Vec3f &up);
		static Matrix4x4f cameraDirectionMatrixAutoUp(const Vec3f &direction); // Chooses up such that it doesn't lock up
		static Matrix4x4f identityMatrix();

		static Matrix4x4f perspectiveMatrix(float fovY, float aspectRatio, float zNear, float zFar);
		static Matrix4x4f orthoMatrix(float left, float right, float bottom, float top, float zNear, float zFar);

		Vec4f operator*(const Vec4f &vec) const;
		Vec3f operator*(const Vec3f &vec) const;
	};
}