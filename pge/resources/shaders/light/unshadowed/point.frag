#version 400

uniform sampler2D pgeGBufferPosition;
uniform sampler2D pgeGBufferNormal;
uniform sampler2D pgeGBufferColor;

layout(shared) uniform pgePointLight {
	vec3 pgePointLightPosition;
	vec3 pgePointLightColor;
	float pgePointLightRange;
	float pgePointLightRangeInv;
};

uniform vec2 pgeGBufferSizeInv;
uniform vec3 pgeAttenuation;

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

	float strength = fallOff / (pgeAttenuation.x + pgeAttenuation.y * distance + pgeAttenuation.z * distance * distance);

	vec4 color = texture(pgeGBufferColor, texCoord);

	if(color.a == 0.0) // No specular
		pgeOutputColor = vec4(color.rgb * strength * lambert * pgePointLightColor, 1.0);
	else {
		vec3 lightRay = reflect(-lightDirection, viewNormal.xyz);
		float specularIntensity = strength * pow(max(0.0, dot(lightRay, normalize(-viewPosition))), viewNormal.a);

		pgeOutputColor = vec4(color.rgb * strength * lambert * pgePointLightColor + color.a * specularIntensity * pgePointLightColor, 1.0);
	}
}