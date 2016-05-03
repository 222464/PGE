#version 400

const int maxNumCascades = 3;

uniform sampler2D pgeGBufferPosition;
uniform sampler2D pgeGBufferNormal;
uniform sampler2D pgeGBufferColor;

uniform sampler2D pgeNoiseMap;

uniform sampler2DShadow pgeCascadeShadowMaps[maxNumCascades];

layout(shared) uniform pgeDirectionalLightShadowed {
	vec3 pgeDirectionalLightColor;
	vec3 pgeDirectionalLightDirection;
	int pgeNumCascades;
	float pgeSplitDistances[maxNumCascades];
	mat4 pgeLightBiasViewProjections[maxNumCascades];
};

uniform vec2 pgeGBufferSizeInv;
uniform vec3 pgeAttenuation;

const int pgeNumSamples = 4;
const float pgeNumSamplesInv = 1.0 / pgeNumSamples;
const float pgeSampleOffset = 0.0009;

out vec4 pgeOutputColor;

void main() {
	vec2 texCoord = gl_FragCoord.xy * pgeGBufferSizeInv;

	vec4 viewNormal = texture(pgeGBufferNormal, texCoord);

	float normalLength = length(viewNormal.xyz);

	float lambert = dot(-pgeDirectionalLightDirection, viewNormal.xyz) * normalLength + 1.0 - normalLength;

	viewNormal.xyz /= normalLength;

	if (lambert <= 0.0) {
		pgeOutputColor = vec4(0.0, 0.0, 0.0, 1.0);
		return;
	}

	float shadowFactor = 1.0;

	vec3 viewPosition = texture(pgeGBufferPosition, texCoord).xyz;

	for (int i = 0; i < pgeNumCascades; i++) {
		if (-viewPosition.z < pgeSplitDistances[i]) {
			// Perform shadowed lighting
			vec4 shadowCoord = pgeLightBiasViewProjections[i] * vec4(viewPosition, 1.0);
			shadowCoord.xy /= shadowCoord.w;
			shadowCoord.z -= 0.0006 + i * 0.0006; // Increase bias with distance (cascade)

			shadowFactor = 0.0;

			float x, y;

			for(int j = 0; j < pgeNumSamples; j++)
				shadowFactor += texture(pgeCascadeShadowMaps[i], vec3(shadowCoord.xy + pgeSampleOffset * (texture(pgeNoiseMap, vec2(j * 0.8475)).xy * 2.0 - 1.0), shadowCoord.z));
					
			shadowFactor *= pgeNumSamplesInv;

			break;
		}
	}

	vec4 color = texture(pgeGBufferColor, texCoord);

	// Specular
	if(color.a == 0.0) // No specular
		pgeOutputColor = vec4(color.rgb * lambert * shadowFactor * pgeDirectionalLightColor, 1.0);
	else { // Specular
		vec3 lightRay = reflect(pgeDirectionalLightDirection, viewNormal.xyz);
		float specularIntensity = pow(max(0.0, dot(lightRay, normalize(-viewPosition))), viewNormal.a);

		pgeOutputColor = vec4(color.rgb * lambert * shadowFactor * pgeDirectionalLightColor + color.a * specularIntensity * pgeDirectionalLightColor, 1.0);
	}
}