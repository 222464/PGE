#version 400

uniform sampler2D pgeDiffuseMap;

uniform sampler2D pgeNoiseMap;

uniform float pgeShininess = 96.0;

smooth in vec2 texCoord;
smooth in float fadeFactor;

out vec4 pgeOutputColor;

void main() {
	float noise = (texture(pgeNoiseMap, texCoord * 2.0).r + 1.0) * fadeFactor;

	if(noise > 0.9)
		discard;

	// Alpha testing
	vec4 diffuseTexColor = texture(pgeDiffuseMap, texCoord);

	if(diffuseTexColor.a < 0.5)
		discard;

	pgeOutputColor = vec4(1.0, 1.0, 1.0, 1.0);
}