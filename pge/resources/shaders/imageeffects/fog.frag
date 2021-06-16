#version 400

uniform sampler2D pgeGBufferPosition;

uniform vec2 pgeSizeInv;

uniform vec3 pgeFogColor;
uniform float pgeFogStartDistance;

out vec4 pgeOutputColor;

void main() {
    vec2 coord = gl_FragCoord.xy * pgeSizeInv;

    // Get color
    pgeOutputColor = vec4(vec3(1.0 - min(1.0, max(0.0, -pgeFogColor.r * texture(pgeGBufferPosition, coord).z - pgeFogStartDistance))), 1.0);
}
