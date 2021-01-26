#version 400

uniform sampler2D pgeDiffuseMap;

smooth in vec2 texCoord;

out vec4 pgeOutputColor;

void main() {
	if (texture(pgeDiffuseMap, texCoord).a < 0.5)
		discard;

	pgeOutputColor = vec4(1.0);
}
