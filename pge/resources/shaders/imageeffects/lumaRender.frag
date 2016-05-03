#version 400

uniform sampler2D pgeScene;

uniform vec2 pgeSizeInv;

out vec4 pgeOutputColor;

void main() {
	vec2 coord = gl_FragCoord.xy * pgeSizeInv;

	vec3 color = texture(pgeScene, coord).rgb;

	pgeOutputColor = vec4(color, dot(color, vec3(0.299, 0.587, 0.114)));
}