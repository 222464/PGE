#version 400

uniform sampler2D pgeGBufferPosition;
uniform sampler2D pgeGBufferNormal;

uniform sampler2D pgeNoiseMap;

uniform vec2 pgeSizeInv;

uniform float pgeRadius;
uniform float pgeStrength;

uniform mat4 pgeProjectionMatrix;

out vec4 pgeOutputColor;

void main() {
    const int pgeSampleKernelSize = 8; // 8 sphere samples
    const float pgeSampleKernelSizeInv = 1.0 / pgeSampleKernelSize;

    const float pgeNoiseScale = 431.8293;

    const vec3 pgeSampleKernal[8] = vec3[8] (
        /*vec3(-0.71139721306844, 0.090695798894178, 0.4135832902777),
        vec3(0.14465329259179, -0.034248374246736, 0.89409288765134),
        vec3(0.58360584008039, 0.44579354263549, 0.47210918149907),
        vec3(0.047548117268109, 0.0030572801751948, 0.10299966569938),
        vec3(-0.21051528102914, -0.069836323762254, 0.11384665140169),
        vec3(0.097336485439401, -0.01082057009773, 0.035590220164099),
        vec3(-0.31928118029602, -0.079396180775845, 0.5184908904021),
        vec3(0.24461479160575, 0.25759929246472, 0.59937776422696),*/
        vec3(-0.17906078187744, -0.53807070920131, 0.31131590821295),
        vec3(0.11925984563736, 0.12737879026415, 0.32825843762036),
        vec3(0.23396103061314, 0.14105650025399, 0.89225951550769),
        vec3(0.015711929311137, -0.14279639826434, 0.27672856343535),
        vec3(0.24737787541745, -0.19831704566618, 0.79865570368882),
        vec3(0.19465182088177, 0.19578779305265, 0.35939170086383),
        vec3(-0.021866072947625, -0.018960301488754, 0.11786785239381),
        vec3(-0.016971704335328, -0.055696815075632, 0.13888215827939)
    );

    vec2 coord = gl_FragCoord.xy * pgeSizeInv;

    vec3 position = texture(pgeGBufferPosition, coord).xyz;
    vec3 normal = texture(pgeGBufferNormal, coord).xyz;

    float normalLength = length(normal);

    normal /= normalLength;

    float occlusion = 0.0;

    vec3 random = texture(pgeNoiseMap, coord * pgeNoiseScale).xyz * 2.0 - 1.0;
    vec3 tangent = normalize(random - normal * dot(random, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 tbn = mat3(tangent, bitangent, normal);

    for (int i = 0; i < pgeSampleKernelSize; i++) {
        vec3 occSample = tbn * pgeSampleKernal[i];
        occSample = occSample * pgeRadius + position;
  
        vec4 offset = vec4(occSample, 1.0);
        offset = pgeProjectionMatrix * offset;
        offset.xy /= offset.w;
        offset.xy = offset.xy * 0.5 + 0.5; 

        float sampleZ = texture(pgeGBufferPosition, offset.xy).z;
  
        float rangeCheck = abs(position.z - sampleZ) < pgeRadius ? 1.0 : 0.0;

        occlusion += step(-sampleZ, -occSample.z) * rangeCheck;
    }
 
    pgeOutputColor = vec4(pow(1.0 - normalLength * occlusion * pgeSampleKernelSizeInv, pgeStrength));
}