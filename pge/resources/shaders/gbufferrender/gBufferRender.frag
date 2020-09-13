#version 400

uniform sampler2D pgeDiffuseMap;
uniform sampler2D pgeSpecularMap;
uniform sampler2D pgeShininessMap;
uniform sampler2D pgeEmissiveMap;

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

void main() {
	// Alpha testing
	vec4 diffuseTexColor = texture(pgeDiffuseMap, texCoord);

	if (diffuseTexColor.a < 0.5)
		discard;

	pgeOutputPosition = vec4(viewPosition, texture(pgeEmissiveMap, texCoord).r * pgeEmissiveColor);
	pgeOutputNormal = vec4(normalLength * viewNormal, pgeShininess * texture(pgeShininessMap, texCoord).r);
	pgeOutputColor = vec4(diffuseTexColor.rgb * pgeDiffuseColor, pgeSpecularColor * texture(pgeSpecularMap, texCoord).r);
}
