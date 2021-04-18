#version 400

uniform sampler2D pgeScene;

uniform vec3 pgeColor;

uniform vec2 pgeGBufferSizeInv;

out vec4 pgeOutputColor;

void main() {
    pgeOutputColor = vec4(pgeColor * texture(pgeScene, gl_FragCoord.xy * pgeGBufferSizeInv).rgb, 1.0);
}