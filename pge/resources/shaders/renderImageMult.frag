#version 400

uniform sampler2D pgeScene;
uniform sampler2D pgeColor;
uniform vec2 pgeGBufferSizeInv;

out vec4 pgeOutputColor;

void main() {
	vec2 coord = gl_FragCoord.xy * pgeGBufferSizeInv;
	pgeOutputColor = texture(pgeColor, coord) * texture(pgeScene, coord);
}