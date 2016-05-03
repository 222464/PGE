#version 400

layout(location = 0) in vec3 positionIn;
layout(location = 1) in vec4 rotationIn;
layout(location = 2) in vec3 colorIn;

out vec4 rotationToGeom;
out vec3 colorToGeom;

void main() {
	rotationToGeom = rotationIn;
	colorToGeom = colorIn;

	gl_Position = vec4(positionIn, 1.0);
}
