#version 400

uniform samplerCube trileTexture;
uniform samplerCube trileNormalMap;

uniform float pgeEmissiveColor;

smooth in vec3 viewPosition;
smooth in vec3 viewNormal;
smooth in vec3 viewTangent;
smooth in vec3 viewBitangent;
smooth in vec3 texVec;

layout (location = 0) out vec4 pgeOutputPosition;
layout (location = 1) out vec4 pgeOutputNormal;
layout (location = 2) out vec4 pgeOutputColor;
void main() {
    //mat3 basis = mat3(viewTangent, viewBitangent, viewNormal);

    pgeOutputPosition = vec4(viewPosition, pgeEmissiveColor);
    pgeOutputNormal = vec4(viewNormal, 96.0);//normalize(basis * (texture(trileNormalMap, texVec).rgb * 2.0 - 1.0)), 96.0);
    pgeOutputColor = vec4(texture(trileTexture, texVec).rgb, 0.0);
}
