#version 400

uniform sampler2D pgeGBufferPosition;
uniform sampler2D pgeGBufferNormal;
uniform sampler2D pgeGBufferColor;
uniform sampler2D pgeGBufferEffect;
uniform samplerCube pgeCubeMap;
uniform sampler2D pgeNoiseMap;

uniform vec2 pgeSizeInv;

const float pgeRayStep = 0.2;
const int pgeMaxSteps = 30;
const float pgeSearchDist = pgeRayStep * pgeMaxSteps;
const float pgeSearchDistInv = 1.0 / pgeSearchDist;
const int pgeNumBinarySearchSteps = 3;
const float pgeError = 2.0;
const float pgeEnvMapPower = 0.1;
const int pgeSamples = 1;
const float pgeSamplesInv = 1.0 / pgeSamples;
const float pgeScreenEdgePower = 4.0;
const float pgeZAlignedExtraLength = 8.0;
const float pgeEnvMapIntensity = 0.2;

uniform mat4 pgeProjectionMatrix;
uniform mat3 pgeNormalMatrixInv;

out vec4 pgeOutputColor;

void main() {
	vec2 coord = gl_FragCoord.xy * pgeSizeInv;

	// Samples
	float specular = texture(pgeGBufferColor, coord).a;

	if(specular == 0.0) {
		pgeOutputColor = vec4(0.0, 0.0, 0.0, 1.0);
		return;
	}

	vec3 color = vec3(0.0);

	for (int s = 0; s < pgeSamples; s++) {
		vec3 position = texture(pgeGBufferPosition, coord).xyz;
		vec4 normalAndShininess = texture(pgeGBufferNormal, coord);
		vec3 normal = normalize(normalAndShininess.xyz);

		// Reflection vector
		vec3 reflected = normalize(reflect(normalize(position), normal));

		// Perturb reflection
		reflected += (texture(pgeNoiseMap, coord * 532.4323 + vec2(float(s) * 31.0162)).xyz * 2.0 - 1.0) / normalAndShininess.w;

		reflected = normalize(reflected);

		vec3 positionStep = reflected * pgeRayStep;// * (1.0 + pgeZAlignedExtraLength * abs(dot(reflected, vec3(0.0, 0.0, 1.0)))) * -position.z;

		float distance = length(positionStep) * pgeMaxSteps;

		float error = pgeError * length(positionStep);

		// Ray cast
		vec3 hitPos = position;

		vec4 projectedHitPos;
		vec2 hitCoord;

		for (int i = 0; i < pgeMaxSteps; i++) {
			hitPos += positionStep;
			projectedHitPos = pgeProjectionMatrix * vec4(hitPos, 1.0);
			hitCoord = projectedHitPos.xy / projectedHitPos.w * 0.5 + 0.5;

			float depth = texture(pgeGBufferPosition, hitCoord).z;
			//vec3 hitNormal = normalize(texture(pgeGBufferNormal, hitCoord).xyz);

			// && dot(-reflected, hitNormal) > 0.0

			if (hitPos.z < depth && abs(depth - hitPos.z) < error) {
				// Binary search
				positionStep *= 0.5;

				hitPos -= positionStep;
				projectedHitPos = pgeProjectionMatrix * vec4(hitPos, 1.0);
				hitCoord = projectedHitPos.xy / projectedHitPos.w * 0.5 + 0.5;

				for (int j = 0; j < pgeNumBinarySearchSteps; j++) {
					depth = texture(pgeGBufferPosition, hitCoord).z;
					//hitNormal = normalize(texture(pgeGBufferNormal, hitCoord).xyz);

					// && dot(-reflected, hitNormal) > 0.0
					if (!(hitPos.z < depth && depth - hitPos.z < error)) {
						hitPos += positionStep;
						projectedHitPos = pgeProjectionMatrix * vec4(hitPos, 1.0);
						hitCoord = projectedHitPos.xy / projectedHitPos.w * 0.5 + 0.5;
					}

					positionStep *= 0.5;

					hitPos -= positionStep;
					projectedHitPos = pgeProjectionMatrix * vec4(hitPos, 1.0);
					hitCoord = projectedHitPos.xy / projectedHitPos.w * 0.5 + 0.5;
				}

				break;
			}
		}

		vec2 dCoords = 2.0 * abs(vec2(0.5, 0.5) - hitCoord);

		float screenEdgefactor = pow(clamp(1.0 - dCoords.x, 0.0, 1.0) * clamp(1.0 - dCoords.y, 0.0, 1.0), pgeScreenEdgePower);

		vec3 hitNormal = normalize(texture(pgeGBufferNormal, hitCoord).xyz);

		float ssrStrength;
		
		//if (dot(-reflected, hitNormal) < 0.0)
		//	ssrStrength = 0.0;
		//else
			ssrStrength = pow(clamp(((distance - length(hitPos - position)) / distance) * (1.0 - reflected.z) * screenEdgefactor * max(0.0, dot(hitNormal, -reflected)), 0.0, 1.0), pgeEnvMapPower);

		color += texture(pgeGBufferEffect, hitCoord).rgb * ssrStrength + texture(pgeCubeMap, pgeNormalMatrixInv * reflected).rgb * (1.0 - ssrStrength) * pgeEnvMapIntensity;
	}

	vec3 baseColor = texture(pgeGBufferColor, coord).rgb;

	// Get color
	pgeOutputColor = vec4(specular * baseColor * color * pgeSamplesInv, 1.0);
}