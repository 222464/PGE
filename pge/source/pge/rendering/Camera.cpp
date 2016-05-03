#include <pge/rendering/Camera.h>

using namespace pge;

void Camera::extractFrustum() {
	_frustum.extractFromMatrix(_projectionViewMatrix);
	_frustum.calculateCorners(_projectionViewInverseMatrix);
}

void Camera::fullUpdate() {
	updateViewMatrix();
	updateViewInverseMatrix();
	updateNormalMatrix();
	updateProjectionViewMatrix();
	updateProjectionViewInverseMatrix();

	extractFrustum();
}