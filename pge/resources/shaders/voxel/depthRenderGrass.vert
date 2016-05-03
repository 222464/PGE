#version 400

uniform mat4 pgeViewModel;
uniform mat4 pgeProjectionViewModel;
uniform mat3 pgeNormal;

uniform float pgeWaveTime;
uniform float pgeCompletelyVisibleDistance;
uniform float pgeInvFadeRange; // = 1.0 / (pgeCompletelyInvisibleDistance - pgeCompletelyVisibleDistance)
uniform float pgeAirWavDistMultInv = 0.4;
uniform float pgeWaveDistance = 0.04;

layout(location = 0) in vec3 positionIn;
layout(location = 2) in vec2 texCoordIn;

smooth out vec2 texCoord;
smooth out float fadeFactor;

void main() {
	vec3 position = positionIn;

	position.xz += vec2(sin(pgeWaveTime + position.x * pgeAirWavDistMultInv), cos(pgeWaveTime + position.z * pgeAirWavDistMultInv)) * pgeWaveDistance * texCoordIn.y;

	vec3 viewPosition = (pgeViewModel * vec4(position, 1.0)).xyz;
	texCoord = texCoordIn;

	gl_Position = pgeProjectionViewModel * vec4(position, 1.0);

	// Distance from camera
	float dist = length(viewPosition);

	fadeFactor = max(0.0, (dist - pgeCompletelyVisibleDistance) * pgeInvFadeRange);
}
