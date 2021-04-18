#version 400

uniform mat4 pgeViewModel;
uniform mat4 pgeProjectionViewModel;
uniform mat3 pgeNormal;

layout(location = 0) in vec3 positionIn;
layout(location = 1) in vec3 normalIn;

smooth out vec3 viewPosition;
smooth out vec3 viewNormal;
smooth out vec3 worldPosition;
smooth out vec3 worldNormal;

void main() {
    viewPosition = (pgeViewModel * vec4(positionIn, 1.0)).xyz;
    viewNormal = normalize(pgeNormal * normalIn);

    worldPosition = positionIn;
    worldNormal = normalIn;

    gl_Position = pgeProjectionViewModel * vec4(positionIn, 1.0);
}
