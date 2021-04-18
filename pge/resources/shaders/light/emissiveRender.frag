#version 400

uniform sampler2D pgeGBufferPosition;
uniform sampler2D pgeGBufferColor;

uniform vec2 pgeGBufferSizeInv;

out vec4 pgeOutputColor;

void main() {
    vec2 texCoord = gl_FragCoord.xy * pgeGBufferSizeInv;
    
    pgeOutputColor = texture(pgeGBufferColor, texCoord) * vec4(texture(pgeGBufferPosition, texCoord).a);
}