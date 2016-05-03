#version 400

uniform sampler2D pgeGBufferPosition;

uniform vec2 pgeSizeInv;

uniform vec4 pgeLightProjectedPosition;
uniform vec3 pgeLightViewPosition;

uniform vec3 pgeColor;

const int pgeSteps = 60;

const float pgeDecay = 0.95;

const float pgeStrengthScalar = 0.2;
const float pgeFalloffScalar = 800.0;

out vec4 pgeOutputColor;

void main() {
	vec2 coord = gl_FragCoord.xy * pgeSizeInv;

	vec2 sampleCoord = pgeLightProjectedPosition.xy / pgeLightProjectedPosition.w * 0.5 + 0.5;

	vec2 deltaCoord = (coord - sampleCoord) * normalize(1.0 / pgeSizeInv);

	deltaCoord /= float(pgeSteps);

	float illum = 1.0;

	float strength = 0.0;

	for (int i = 0; i < pgeSteps; i++) {
		sampleCoord += deltaCoord;

		float depth = -texture(pgeGBufferPosition, sampleCoord).z;

		strength +=  depth > -pgeLightViewPosition.z ? illum : 0.0;

		illum *= pgeDecay;
	}

	pgeOutputColor = vec4(vec3(strength * pgeStrengthScalar / (1.0 + length(deltaCoord) * pgeFalloffScalar)) * pgeColor, 1.0);
}