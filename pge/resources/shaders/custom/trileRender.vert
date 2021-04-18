#version 400

uniform mat4 pgeViewModel;
uniform mat4 pgeProjectionViewModel;
uniform mat3 pgeNormal;

layout(location = 0) in vec3 positionIn;
layout(location = 1) in vec3 normalIn;

smooth out vec3 viewPosition;
smooth out vec3 viewNormal;
smooth out vec3 viewTangent;
smooth out vec3 viewBitangent;
smooth out vec3 texVec;

void main() {
    viewPosition = (pgeViewModel * vec4(positionIn, 1.0)).xyz;

    if (normalIn.y > 0.95 || normalIn.y < -0.95) {
        viewNormal = normalize(pgeNormal * normalIn);
        viewTangent = normalize(pgeNormal * cross(normalIn, vec3(1.0, 0.0, 0.0)));
        viewBitangent = normalize(cross(viewTangent, viewNormal));
    }
    else {
        viewNormal = normalize(pgeNormal * normalIn);
        viewTangent = normalize(pgeNormal * cross(normalIn, vec3(0.0, 1.0, 0.0)));
        viewBitangent = normalize(cross(viewTangent, viewNormal));
    }

    vec3 notFollowingNormalAxes = vec3(1.0, 1.0, 1.0) - abs(normalIn);

    texVec = positionIn * notFollowingNormalAxes + normalIn * 0.5;

    gl_Position = pgeProjectionViewModel * vec4(positionIn, 1.0);
}
