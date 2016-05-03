#version 400

uniform sampler2D pgeGBufferPosition;
uniform sampler2D pgeGBufferNormal;
uniform sampler2D pgeGBufferColor;

uniform sampler2D pgeNoiseMap;

uniform samplerCubeShadow pgeShadowMap;

layout(shared) uniform pgePointLightShadowed {
	vec3 pgePointLightPosition;
	vec3 pgePointLightColor;
	float pgePointLightRange;
	float pgePointLightRangeInv;

	mat4 pgeToLightSpace;

	// Instead of using full projection matrix, use only elements of it that are needed
	float pgeProj22;
	float pgeProj23;
};

uniform vec2 pgeGBufferSizeInv;
uniform vec3 pgeAttenuation;

const int pgeNumSamples = 16;
const float pgeNumSamplesInv = 1.0 / pgeNumSamples;
const float pgeSampleOffset = 0.05;

out vec4 pgeOutputColor;

void main() {
	vec2 texCoord = gl_FragCoord.xy * pgeGBufferSizeInv;

	vec3 viewPosition = texture(pgeGBufferPosition, texCoord).xyz;

	vec3 lightDirection = pgePointLightPosition - viewPosition;
	float distance = length(lightDirection);
	
	lightDirection /= distance;

	vec4 viewNormal = texture(pgeGBufferNormal, texCoord);

	float normalLength = length(viewNormal.xyz);

	float lambert = dot(lightDirection, viewNormal.xyz) * normalLength + 1.0 - normalLength;

	viewNormal.xyz /= normalLength;

	if(lambert <= 0.0) {
		pgeOutputColor = vec4(0.0, 0.0, 0.0, 1.0);
		return;
	}

	float fallOff = (pgePointLightRange - distance) * pgePointLightRangeInv;

	if(fallOff <= 0.0) {
		pgeOutputColor = vec4(0.0, 0.0, 0.0, 1.0);
		return;
	}

	vec3 lightSpacePosition = (pgeToLightSpace * vec4(viewPosition, 1.0)).xyz;
	vec3 absLightSpacePosition = abs(lightSpacePosition);
	float maxDist = max(absLightSpacePosition.x, max(absLightSpacePosition.y, absLightSpacePosition.z));

	float depth = (pgeProj22 * -maxDist + pgeProj23) / maxDist * 0.5 + 0.5;

	// Bias
	depth -= 0.005;

	float shadow = 0.0;
	
	for (int i = 0; i < pgeNumSamples; i++) {
		vec3 sampleRay = lightSpacePosition + pgeSampleOffset * (texture(pgeNoiseMap, vec2(i * 0.8475)).xyz * 2.0 - 1.0);

		shadow += texture(pgeShadowMap, vec4(sampleRay, depth));
	}

	shadow *= pgeNumSamplesInv;

	float strength = shadow * fallOff / (pgeAttenuation.x + pgeAttenuation.y * distance + pgeAttenuation.z * distance * distance);

	vec4 color = texture(pgeGBufferColor, texCoord);

	if(color.a == 0.0) // No specular
		pgeOutputColor = vec4(color.rgb * strength * lambert * pgePointLightColor, 1.0);
	else {
		vec3 lightRay = reflect(-lightDirection, viewNormal.xyz);
		float specularIntensity = strength * pow(max(0.0, dot(lightRay, normalize(-viewPosition))), viewNormal.a);

		pgeOutputColor = vec4(color.rgb * strength * lambert * pgePointLightColor + color.a * specularIntensity * pgePointLightColor, 1.0);
	}
}