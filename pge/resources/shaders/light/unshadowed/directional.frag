#version 400

uniform sampler2D pgeGBufferPosition;
uniform sampler2D pgeGBufferNormal;
uniform sampler2D pgeGBufferColor;

layout(shared) uniform pgeDirectionalLight {
	vec3 pgeDirectionalLightColor;
	vec3 pgeDirectionalLightDirection;
};

uniform vec2 pgeGBufferSizeInv;
uniform vec3 pgeAttenuation;

out vec4 pgeOutputColor;

void main() {
	vec2 texCoord = gl_FragCoord.xy * pgeGBufferSizeInv;

	vec4 viewNormal = texture(pgeGBufferNormal, texCoord);

	float normalLength = length(viewNormal.xyz);

	float lambert = dot(-pgeDirectionalLightDirection, viewNormal.xyz) * normalLength + 1.0 - normalLength;

	viewNormal.xyz /= normalLength;

	if(lambert <= 0.0) {
		pgeOutputColor = vec4(0.0, 0.0, 0.0, 1.0);
		return;
	}

	vec4 color = texture(pgeGBufferColor, texCoord);

	// Specular
	if(color.a == 0.0) // No specular
		pgeOutputColor = vec4(color.rgb * lambert * pgeDirectionalLightColor, 1.0);
	else { // Specular
		vec3 viewPosition = texture(pgeGBufferPosition, texCoord).xyz;

		vec3 lightRay = reflect(pgeDirectionalLightDirection, viewNormal.xyz);
		float specularIntensity = pow(max(0.0, dot(lightRay, normalize(-viewPosition))), viewNormal.a);

		pgeOutputColor = vec4(color.rgb * lambert * pgeDirectionalLightColor + color.a * specularIntensity * pgeDirectionalLightColor, 1.0);
	}
}