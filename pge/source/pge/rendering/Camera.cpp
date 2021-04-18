#include "Camera.h"

using namespace pge;

void Camera::extractFrustum() {
    frustum.extractFromMatrix(projectionViewMatrix);
    frustum.calculateCorners(projectionViewInverseMatrix);
}

void Camera::fullUpdate() {
    updateViewMatrix();
    updateViewInverseMatrix();
    updateNormalMatrix();
    updateProjectionViewMatrix();
    updateProjectionViewInverseMatrix();

    extractFrustum();
}