#version 400

uniform sampler2D pgeScene;
uniform sampler2D pgeGBufferPosition;
uniform vec2 pgeSizeInv;

uniform float pgeBlurSize;

uniform float pgeFocalDistance;
uniform float pgeFocalRange;

const float pgeBlurPower = 2.0;
const float pgeBlurStrengthScalar = 200.0;

out vec4 pgeOutputColor;

void main() {
    vec3 sum = vec3(0.0);

    vec2 coord = gl_FragCoord.xy * pgeSizeInv;

    float distFromFocal = abs(pgeFocalDistance + texture(pgeGBufferPosition, coord).z);

    float blurSize = min(pgeBlurSize, pgeBlurStrengthScalar * pow(pgeBlurSize * max(0.0, distFromFocal - pgeFocalRange), pgeBlurPower));

    sum += texture(pgeScene, vec2(coord.x, coord.y - 4.0 * blurSize)).rgb * 0.05;
    sum += texture(pgeScene, vec2(coord.x, coord.y - 3.0 * blurSize)).rgb * 0.09;
    sum += texture(pgeScene, vec2(coord.x, coord.y - 2.0 * blurSize)).rgb * 0.12;
    sum += texture(pgeScene, vec2(coord.x, coord.y - blurSize)).rgb * 0.15;
    sum += texture(pgeScene, vec2(coord.x, coord.y)).rgb * 0.16;
    sum += texture(pgeScene, vec2(coord.x, coord.y + blurSize)).rgb * 0.15;
    sum += texture(pgeScene, vec2(coord.x, coord.y + 2.0 * blurSize)).rgb * 0.12;
    sum += texture(pgeScene, vec2(coord.x, coord.y + 3.0 * blurSize)).rgb * 0.09;
    sum += texture(pgeScene, vec2(coord.x, coord.y + 4.0 * blurSize)).rgb * 0.05;
 
    pgeOutputColor = vec4(sum, 1.0);
}