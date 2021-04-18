#version 400

uniform sampler2D pgeScene;
uniform vec2 pgeGBufferSizeInv;

out vec4 pgeOutputColor;

void main() {
    pgeOutputColor = texture(pgeScene, gl_FragCoord.xy * pgeGBufferSizeInv);
}