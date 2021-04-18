#version 400

uniform mat4 pgeViewModel;
uniform mat4 pgeProjectionViewModel;
uniform mat3 pgeNormal;

layout(location = 0) in vec3 positionIn;

void main() {
    gl_Position = pgeProjectionViewModel * vec4(positionIn, 1.0);
}
