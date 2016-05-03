#version 400

smooth in vec3 viewPosition;
smooth in vec3 viewNormal;
in vec3 color;

layout (location = 0) out vec4 pgeOutputPosition;
layout (location = 1) out vec4 pgeOutputNormal;
layout (location = 2) out vec4 pgeOutputColor;

void main() {
	pgeOutputPosition = vec4(viewPosition, 0.15);
	pgeOutputNormal = vec4(viewNormal, 96.0);
	pgeOutputColor = vec4(color, 0.0);
}