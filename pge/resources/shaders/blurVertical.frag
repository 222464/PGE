#version 400

uniform sampler2D pgeScene;
uniform vec2 pgeSizeInv;

uniform float pgeBlurSize;

out vec4 pgeOutputColor;

void main() {
    vec3 sum = vec3(0.0);

    vec2 coord = gl_FragCoord.xy * pgeSizeInv;

    sum += texture(pgeScene, vec2(coord.x, coord.y - 4.0 * pgeBlurSize)).rgb * 0.05;
    sum += texture(pgeScene, vec2(coord.x, coord.y - 3.0 * pgeBlurSize)).rgb * 0.09;
    sum += texture(pgeScene, vec2(coord.x, coord.y - 2.0 * pgeBlurSize)).rgb * 0.12;
    sum += texture(pgeScene, vec2(coord.x, coord.y - pgeBlurSize)).rgb * 0.15;
    sum += texture(pgeScene, vec2(coord.x, coord.y)).rgb * 0.16;
    sum += texture(pgeScene, vec2(coord.x, coord.y + pgeBlurSize)).rgb * 0.15;
    sum += texture(pgeScene, vec2(coord.x, coord.y + 2.0 * pgeBlurSize)).rgb * 0.12;
    sum += texture(pgeScene, vec2(coord.x, coord.y + 3.0 * pgeBlurSize)).rgb * 0.09;
    sum += texture(pgeScene, vec2(coord.x, coord.y + 4.0 * pgeBlurSize)).rgb * 0.05;
 
    pgeOutputColor = vec4(sum, 1.0);
}