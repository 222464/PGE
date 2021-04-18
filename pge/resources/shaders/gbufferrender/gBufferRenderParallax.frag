#version 400

uniform sampler2D pgeDiffuseMap;
uniform sampler2D pgeSpecularMap;
uniform sampler2D pgeShininessMap;
uniform sampler2D pgeEmissiveMap;
uniform sampler2D pgeNormalMap;
uniform sampler2D pgeHeightMap;

layout(shared) uniform pgeMaterial {
    vec3 pgeDiffuseColor;
    float pgeSpecularColor;
    float pgeShininess;
    float pgeEmissiveColor;
    float pgeHeightMapScale;
};

smooth in vec3 viewPosition;
smooth in vec3 viewNormal;
smooth in vec2 texCoord;
smooth in float normalLength;

layout (location = 0) out vec4 pgeOutputPosition;
layout (location = 1) out vec4 pgeOutputNormal;
layout (location = 2) out vec4 pgeOutputColor;

// Undefine to use just parallax mapping
#define PARALLAX_OCCLUSION_MAPPING

mat3 calculateBasis(inout vec3 tangentEyePos) {
    vec3 q0 = dFdx(viewPosition);
    vec3 q1 = dFdy(viewPosition);
    vec2 st0 = dFdx(texCoord);
    vec2 st1 = dFdy(texCoord);
    vec3 S = normalize(q0 * st1.t - q1 * st0.t);
    vec3 T = normalize(-q0 * st1.s + q1 * st0.s);

    tangentEyePos = vec3(dot(viewPosition, S), dot(viewPosition, T), dot(-viewPosition, viewNormal));

    return mat3(T, S, viewNormal);
}

void linearRayCast(sampler2D heightMap, float numSteps, inout vec3 outputPosition, inout vec3 tangentEyeVec) {
    tangentEyeVec /= numSteps;

    float numStepsOneLess = numSteps - 1;

    for(int i = 0; i < numStepsOneLess; i++) { 
        float depth = texture(heightMap, outputPosition.xy).r;

        if(outputPosition.z < depth)
            outputPosition += tangentEyeVec; 
    } 
}

void binarySearchRayCast(sampler2D heightMap, inout vec3 outputPosition, inout vec3 tangentEyeVec) { 
    const int numSteps = 4;

    for(int i = 0; i < numSteps; i++) { 
        float depth = texture(heightMap, outputPosition.xy).r;

        if(outputPosition.z < depth) 
            outputPosition += tangentEyeVec; 
        
        tangentEyeVec *= 0.5; 
        outputPosition -= tangentEyeVec; 
    } 
}

void main() {
    vec3 tangentEyePos;

    mat3 basis = calculateBasis(tangentEyePos);

    float tangentDist = length(tangentEyePos);

    vec3 tangentSpaceEye = tangentEyePos / tangentDist;

#ifdef PARALLAX_OCCLUSION_MAPPING

    float eyeNormalDot = -dot(normalize(viewPosition), viewNormal);

    tangentSpaceEye *= 1.0 / tangentSpaceEye.z;

    tangentSpaceEye.xy *= pgeHeightMapScale * eyeNormalDot;

    vec3 heightMapPos = vec3(texCoord, 0.0); 

    float numSteps =  2.0 + min(2.0, eyeNormalDot * 2.0); 

    linearRayCast(pgeHeightMap, numSteps, heightMapPos, tangentSpaceEye);
    
    // Binary search for more precision after boundaries were found with linear search
    binarySearchRayCast(pgeHeightMap, heightMapPos, tangentSpaceEye);

    vec2 displacedTexCoords = heightMapPos.xy;

#else // Plain parallax mapping

    float height = texture2D(pgeHeightMap, gl_TexCoord[0].st).r * pgeHeightMapScale;

    vec2 displacedTexCoords = tangentSpaceEye.xy * height + gl_TexCoord[0].st;

#endif

    vec4 diffuseTexColor = texture2D(pgeDiffuseMap, displacedTexCoords).rgba;

    vec3 perturbedNormal = normalize(basis * (texture(pgeNormalMap, displacedTexCoords).rgb * 2.0 - 1.0));

    pgeOutputPosition = vec4(viewPosition, texture(pgeEmissiveMap, texCoord).r * pgeEmissiveColor);//position - perturbedNormal * height;
    pgeOutputNormal = vec4(normalLength * perturbedNormal, pgeShininess * texture(pgeShininessMap, texCoord).r);
    pgeOutputColor = vec4(diffuseTexColor.rgb * pgeDiffuseColor, pgeSpecularColor * texture(pgeSpecularMap, texCoord).r);
}