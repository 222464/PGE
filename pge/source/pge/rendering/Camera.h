#pragma once

#include <pge/rendering/SFMLOGL.h>
#include <pge/rendering/culling/Frustum.h>

#include <pge/constructs/Vec3f.h>
#include <pge/constructs/Quaternion.h>
#include <pge/constructs/Matrix4x4f.h>

namespace pge {
	class Camera {
	private:
		Matrix4x4f _viewMatrix;
		Matrix4x4f _viewInverseMatrix;
		Matrix4x4f _projectionViewMatrix;
		Matrix4x4f _projectionViewInverseMatrix;

		Matrix3x3f _normalMatrix;

	public:
		Frustum _frustum;

		Matrix4x4f _projectionMatrix;

		Vec3f _position;
		Quaternion _rotation;

		Camera()
			: _position(0.0f, 0.0f, 0.0f), _rotation(Quaternion::identityMult())
		{
			fullUpdate();
		}

		void updateViewMatrix() {
			_viewMatrix = _rotation.getMatrix() * Matrix4x4f::translateMatrix(-_position);
		}

		void updateViewInverseMatrix() {
			_viewMatrix.inverse(_viewInverseMatrix);
		}

		void updateProjectionViewMatrix() {
			_projectionViewMatrix = _projectionMatrix * _viewMatrix;
		}

		void updateProjectionViewInverseMatrix() {
			_projectionViewMatrix.inverse(_projectionViewInverseMatrix);
		}

		void updateNormalMatrix() {
			Matrix3x3f upperLeftSubmatrixInverse;

			_viewMatrix.getUpperLeftMatrix3x3f().inverse(upperLeftSubmatrixInverse);

			_normalMatrix = upperLeftSubmatrixInverse.transpose();
		}

		const Matrix4x4f &getViewMatrix() const {
			return _viewMatrix;
		}

		const Matrix4x4f &getViewInverseMatrix() const {
			return _viewInverseMatrix;
		}

		const Matrix4x4f &getProjectionViewMatrix() const {
			return _projectionViewMatrix;
		}

		const Matrix4x4f &getProjectionViewInverseMatrix() const {
			return _projectionViewInverseMatrix;
		}

		const Matrix3x3f &getNormalMatrix() const {
			return _normalMatrix;
		}

		void extractFrustum();

		void fullUpdate();
	};
}