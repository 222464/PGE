#version 400

uniform sampler2D pgeScene;
uniform sampler2D pgeGBufferPosition;
uniform sampler2D pgeGBufferNormal;
uniform vec2 pgeSizeInv;

uniform float pgeBlurSize;

const float pgeEdgeFactor = 0.5;

out vec4 pgeOutputColor;

void main() {
	vec3 sum = vec3(0.0);

	vec2 coord = gl_FragCoord.xy * pgeSizeInv;

	vec3 centerNormal = texture(pgeGBufferNormal, coord).rgb;

	vec2 coord0 = vec2(coord.x - 4.0 * pgeBlurSize, coord.y);
	vec2 coord1 = vec2(coord.x - 3.0 * pgeBlurSize, coord.y);
	vec2 coord2 = vec2(coord.x - 2.0 * pgeBlurSize, coord.y);
	vec2 coord3 = vec2(coord.x - pgeBlurSize, coord.y);
	vec2 coord4 = vec2(coord.x, coord.y);
	vec2 coord5 = vec2(coord.x + pgeBlurSize, coord.y);
	vec2 coord6 = vec2(coord.x + 2.0 * pgeBlurSize, coord.y);
	vec2 coord7 = vec2(coord.x + 3.0 * pgeBlurSize, coord.y);
	vec2 coord8 = vec2(coord.x + 4.0 * pgeBlurSize, coord.y);

	vec3 centerColor = texture(pgeScene, coord4).rgb;

	sum += dot(centerNormal, texture(pgeGBufferNormal, coord0).rgb) > pgeEdgeFactor ? (texture(pgeScene, coord0).rgb * 0.05) : (centerColor * 0.05);
	sum += dot(centerNormal, texture(pgeGBufferNormal, coord1).rgb) > pgeEdgeFactor ? (texture(pgeScene, coord1).rgb * 0.09) : (centerColor * 0.09);
	sum += dot(centerNormal, texture(pgeGBufferNormal, coord2).rgb) > pgeEdgeFactor ? (texture(pgeScene, coord2).rgb * 0.12) : (centerColor * 0.12);
	sum += dot(centerNormal, texture(pgeGBufferNormal, coord3).rgb) > pgeEdgeFactor ? (texture(pgeScene, coord3).rgb * 0.15) : (centerColor * 0.15);
	sum += dot(centerNormal, texture(pgeGBufferNormal, coord4).rgb) > pgeEdgeFactor ? (texture(pgeScene, coord4).rgb * 0.16) : (centerColor * 0.16);
	sum += dot(centerNormal, texture(pgeGBufferNormal, coord5).rgb) > pgeEdgeFactor ? (texture(pgeScene, coord5).rgb * 0.15) : (centerColor * 0.15);
	sum += dot(centerNormal, texture(pgeGBufferNormal, coord6).rgb) > pgeEdgeFactor ? (texture(pgeScene, coord6).rgb * 0.12) : (centerColor * 0.12);
	sum += dot(centerNormal, texture(pgeGBufferNormal, coord7).rgb) > pgeEdgeFactor ? (texture(pgeScene, coord7).rgb * 0.09) : (centerColor * 0.09);
	sum += dot(centerNormal, texture(pgeGBufferNormal, coord8).rgb) > pgeEdgeFactor ? (texture(pgeScene, coord8).rgb * 0.05) : (centerColor * 0.05);
 
	pgeOutputColor = vec4(sum, 1.0);
}