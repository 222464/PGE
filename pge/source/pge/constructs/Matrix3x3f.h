#pragma once

#include "Vec3f.h"
#include "Vec2f.h"
#include <vector>
#include <array>
#include <assert.h>

/*
    Different from mathematics standard:

    i means column
    j means row
*/

// 1D array stores the matrix in column-major order, like OpenGL
namespace pge {
    class Matrix3x3f {
    public:
        static const float directionMatrixNormalizationTolerance;

        std::array<float, 9> elements;

        Matrix3x3f()
        {}

        Matrix3x3f(std::vector<float> sourceArray) {
            for (size_t i = 0; i < 9; i++)
                elements[i] = sourceArray[i];
        }

        Matrix3x3f(std::array<float, 9> sourceArray) {
            elements = sourceArray;
        }

        float &operator[](int i) {
            return elements[i];
        }

        Matrix3x3f operator*(const Matrix3x3f &other) const;

        Matrix3x3f operator*=(const Matrix3x3f &other) {
            return (*this) = (*this) * other;
        }

        bool operator==(const Matrix3x3f &other) const;

        bool operator!=(const Matrix3x3f &other) const {
            return !((*this) == other);
        }

        void set(int i, int j, float val) {
            assert(i >= 0 && j >= 0 && i < 3 && j < 3);

            elements[3 * i + j] = val; // Row-major would be i + 3 * j
        }

        float get(int i, int j) const {
            assert(i >= 0 && j >= 0 && i < 3 && j < 3);

            return elements[3 * i + j]; // Row-major would be i + 3 * j
        }

        void setIdentity();
        Matrix3x3f transpose() const;

        float determinant() const;

        void getUBOPadded(std::array<float, 12> &data) const;

        // Returns false if there is no inverse, true if there is one
        // Warning! Expensive function!
        bool inverse(Matrix3x3f &inverse) const;

        // Transformation matrix generators
        static Matrix3x3f scaleMatrix(const Vec2f &scale);
        static Matrix3x3f translateMatrix(const Vec2f translation);
        static Matrix3x3f rotateMatrix(float angle);
        static Matrix3x3f identityMatrix();

        Vec3f operator*(const Vec3f &vec) const;
        Vec2f operator*(const Vec2f &vec) const;
    };
}