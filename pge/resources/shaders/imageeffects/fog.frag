#version 400

uniform sampler2D pgeGBufferPosition;

uniform vec2 pgeSizeInv;

uniform vec3 pgeFogColor;
uniform float pgeFogStartDistance;

out vec4 pgeOutputColor;

void main() {
    vec2 coord = gl_FragCoord.xy * pgeSizeInv;

    // Get color
    pgeOutputColor = vec4(pgeFogColor * max(0.0, -texture(pgeGBufferPosition, coord).z - pgeFogStartDistance), 1.0);
}