#pragma once

#include "SFMLOGL.h"
#include "culling/Frustum.h"

#include "../constructs/Vec3f.h"
#include "../constructs/Quaternion.h"
#include "../constructs/Matrix4x4f.h"

namespace pge {
	class Camera {
	private:
		Matrix4x4f viewMatrix;
		Matrix4x4f viewInverseMatrix;
		Matrix4x4f projectionViewMatrix;
		Matrix4x4f projectionViewInverseMatrix;

		Matrix3x3f normalMatrix;

	public:
		Frustum frustum;

		Matrix4x4f projectionMatrix;

		Vec3f position;
		Quaternion rotation;

		Camera()
			: position(0.0f, 0.0f, 0.0f), rotation(Quaternion::identityMult())
		{
			fullUpdate();
		}

		void updateViewMatrix() {
			viewMatrix = rotation.getMatrix() * Matrix4x4f::translateMatrix(-position);
		}

		void updateViewInverseMatrix() {
			viewMatrix.inverse(viewInverseMatrix);
		}

		void updateProjectionViewMatrix() {
			projectionViewMatrix = projectionMatrix * viewMatrix;
		}

		void updateProjectionViewInverseMatrix() {
			projectionViewMatrix.inverse(projectionViewInverseMatrix);
		}

		void updateNormalMatrix() {
			Matrix3x3f upperLeftSubmatrixInverse;

			viewMatrix.getUpperLeftMatrix3x3f().inverse(upperLeftSubmatrixInverse);

			normalMatrix = upperLeftSubmatrixInverse.transpose();
		}

		const Matrix4x4f &getViewMatrix() const {
			return viewMatrix;
		}

		const Matrix4x4f &getViewInverseMatrix() const {
			return viewInverseMatrix;
		}

		const Matrix4x4f &getProjectionViewMatrix() const {
			return projectionViewMatrix;
		}

		const Matrix4x4f &getProjectionViewInverseMatrix() const {
			return projectionViewInverseMatrix;
		}

		const Matrix3x3f &getNormalMatrix() const {
			return normalMatrix;
		}

		void extractFrustum();

		void fullUpdate();
	};
}