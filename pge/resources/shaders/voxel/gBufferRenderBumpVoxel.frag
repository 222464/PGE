#version 400

uniform sampler2DArray pgeDiffuseMapArray;
uniform sampler2DArray pgeNormalMapArray;

smooth in vec3 viewPosition;
smooth in vec3 viewNormal;
smooth in vec3 worldPosition;
smooth in vec3 worldNormal;

uniform float pgeSpecular = 1.0;
uniform float pgeShininess = 1.0;
uniform float pgeTextureStretchScalar = 0.3;

uniform float pgeSideSeparationExponent = 7.0;

layout (location = 0)  out vec4 pgeOutputPosition;
layout (location = 1)  out vec4 pgeOutputNormal;
layout (location = 2)  out vec4 pgeOutputColor;

mat3 calculateBasis(vec2 texCoord) {
    vec3 dp1 = dFdx(viewPosition);
    vec3 dp2 = dFdy(viewPosition);
    vec2 duv1 = dFdx(texCoord);
    vec2 duv2 = dFdy(texCoord);
 
    vec3 dp2perp = cross(dp2, viewNormal);
    vec3 dp1perp = cross(viewNormal, dp1);
    vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;
 
    float invMax = inversesqrt(max(dot(T, T), dot(B, B)));
    return mat3(T * invMax, B * invMax, viewNormal);
}

void main() {
	// Blend factors
	vec2 coord1 = (worldPosition.xz * pgeTextureStretchScalar);
	vec2 coord2 = (worldPosition.xy * pgeTextureStretchScalar);
	vec2 coord3 = (worldPosition.yz * pgeTextureStretchScalar);

	vec4 blend = vec4(
		pow(max(0.0, dot(worldNormal, vec3(0.0, 1.0, 0.0))), pgeSideSeparationExponent), // Top
		pow(abs(dot(worldNormal, vec3(1.0, 0.0, 0.0))), pgeSideSeparationExponent), // XY
		pow(abs(dot(worldNormal, vec3(0.0, 0.0, 1.0))), pgeSideSeparationExponent), // ZY
		pow(max(0.0, dot(worldNormal, vec3(0.0, -1.0, 0.0))), pgeSideSeparationExponent) // Bottom
	);
	
	blend = normalize(blend);

	pgeOutputPosition = vec4(viewPosition, 0.0);

	pgeOutputNormal = vec4(normalize(
		blend.x * calculateBasis(coord1) * (texture(pgeNormalMapArray, vec3(coord1, 0)).rgb * 2.0 - 1.0) +
		blend.y * calculateBasis(coord3) * (texture(pgeNormalMapArray, vec3(coord3, 1)).rgb * 2.0 - 1.0) +
		blend.z * calculateBasis(coord2) * (texture(pgeNormalMapArray, vec3(coord2, 1)).rgb * 2.0 - 1.0) +
		blend.w * calculateBasis(coord1) * (texture(pgeNormalMapArray, vec3(coord1, 2)).rgb * 2.0 - 1.0)), pgeShininess);

	pgeOutputColor = vec4((
		blend.x * texture(pgeDiffuseMapArray, vec3(coord1, 0)).rgb + 
		blend.y * texture(pgeDiffuseMapArray, vec3(coord3, 1)).rgb +
		blend.z * texture(pgeDiffuseMapArray, vec3(coord2, 1)).rgb +
		blend.w * texture(pgeDiffuseMapArray, vec3(coord1, 2)).rgb),
		pgeSpecular);
}