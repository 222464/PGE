#version 400

uniform sampler2D pgeDiffuseMap;
uniform sampler2D pgeSpecularMap;
uniform sampler2D pgeShininessMap;
uniform sampler2D pgeEmissiveMap;
uniform sampler2D pgeNormalMap;

layout(shared) uniform pgeMaterial {
	vec3 pgeDiffuseColor;
	float pgeSpecularColor;
	float pgeShininess;
	float pgeEmissiveColor;
	float pgeHeightMapScale;
};

smooth in vec3 viewPosition;
smooth in vec3 viewNormal;
smooth in vec2 texCoord;
smooth in float normalLength;

layout (location = 0) out vec4 pgeOutputPosition;
layout (location = 1) out vec4 pgeOutputNormal;
layout (location = 2) out vec4 pgeOutputColor;

mat3 calculateBasis() {
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
	// Alpha testing
	vec4 diffuseTexColor = texture(pgeDiffuseMap, texCoord);

	if(diffuseTexColor.a < 0.5)
		discard;

	pgeOutputPosition = vec4(viewPosition, texture(pgeEmissiveMap, texCoord).r * pgeEmissiveColor);
	pgeOutputNormal = vec4(normalLength * normalize(calculateBasis() * (texture(pgeNormalMap, texCoord).rgb * 2.0 - 1.0)), pgeShininess * texture(pgeShininessMap, texCoord).r);
	pgeOutputColor = vec4(diffuseTexColor.rgb * pgeDiffuseColor, pgeSpecularColor * texture(pgeSpecularMap, texCoord).r);
}