#ifndef COMMON_ADT_MATERIAL
#define COMMON_ADT_MATERIAL

//-------------------
// Vertex part
//-------------------

float fixADTUVBorder(float uvComp, float x) {
    const float alphaTextureSize_px = 1024.0;
    const float subPixel = 0.5 / alphaTextureSize_px;

    float epsilon = 0.0001;

    if (x < epsilon)
    uvComp += subPixel;
    if ((1.0 - x) < epsilon)
    uvComp -= subPixel;

    return uvComp ;
}

const float TILESIZE = (1600.0 / 3.0);
const float CHUNKSIZE = TILESIZE / 16.0;
const float UNITSIZE =  CHUNKSIZE / 8.0;

void calcAdtAlphaUV(in int indexInMCNK, in vec3 adtMCNKPos, out vec2 alphaCoords, out vec2 chunkCoords) {
    float iX = mod(indexInMCNK, 17.0);
    float iY = floor(indexInMCNK/17.0);

    if (iX > 8.01) {
        iY = iY + 0.5;
        iX = iX - 8.5;
    }

    vec2 worldPoint_xy = vec2(
        adtMCNKPos.x - iY * UNITSIZE,
        adtMCNKPos.y - iX * UNITSIZE
    );

    // Top left point (17058 17063) becomes (0,0)
    vec2 coordsInAdtIndexSpace = 32.0f * TILESIZE - adtMCNKPos.yx;
    // Add half chunk to do little offset before doing floor in next step
    coordsInAdtIndexSpace += vec2(CHUNKSIZE/2.0);

    vec2 ADTIndex = floor(coordsInAdtIndexSpace / TILESIZE);


    chunkCoords = vec2(iX, iY);

    alphaCoords = ((vec2(32.0f) - ADTIndex) * TILESIZE - worldPoint_xy.yx) / TILESIZE;
    alphaCoords.x = fixADTUVBorder(alphaCoords.x, chunkCoords.x/8.0);
    alphaCoords.y = fixADTUVBorder(alphaCoords.y, chunkCoords.y/8.0);
}

//-------------------
//   Fragment part
//-------------------

const vec2 s_texDir[8] = {
    {-1.0, 0.0},
    {-1.0, 1.0},
    {0.0, 1.0},
    {1.0, 1.0},
    {1.0, 0.0},
    {1.0, -1.0},
    {0.0, -1.0},
    {-1.0, -1.0}
};
const float s_tempTexSpeed[8] = {64.0, 48.0, 32.0, 16.0, 8.0, 4.0, 2.0, 1.0};

float fmod1(float x, float y) {
    return x - (y * trunc(x/y));
}

vec2 transformADTUV(in vec2 uv, in int layer, in float sceneTime, in ivec4 animation_rotationPerLayer, in ivec4 animation_speedPerLayer, in vec4 scaleFactorPerLayer) {
    vec2 translate = vec2(0.0);
    int animation_rotation = animation_rotationPerLayer[layer];
    if (animation_rotation >= 0 && animation_rotation < 8) {
        vec2 transVector = s_texDir[animation_rotation] * (sceneTime *  0.001);
        transVector.xy = transVector.yx; //why?
        transVector.x = fmod1(transVector.x, 64);
        transVector.y = fmod1(transVector.y, 64);

        translate = transVector *
            (1.0f / ((1.0f / -4.1666665f) * s_tempTexSpeed[animation_speedPerLayer[layer]]));
    }
    return (uv * scaleFactorPerLayer[layer]) + translate;
}

void calcADTOrigFragMaterial(
    in vec2 tcLayer0, in vec2 tcLayer1, in vec2 tcLayer2, in vec2 tcLayer3,
    in sampler2D t_Layer0, in sampler2D t_Layer1, in sampler2D t_Layer2, in sampler2D t_Layer3,
    in vec2 tcAlpha,
    in sampler2D t_alphaTex,

    out vec4 matDiffuse
) {
    vec3 alphaBlend = texture( t_alphaTex, tcAlpha).rgb;

    vec4 tex1 = texture(t_Layer0, tcLayer0).rgba;
    vec4 tex2 = texture(t_Layer1, tcLayer1).rgba;
    vec4 tex3 = texture(t_Layer2, tcLayer2).rgba;
    vec4 tex4 = texture(t_Layer2, tcLayer3).rgba;

    matDiffuse = mix(mix(mix(tex1, tex2, alphaBlend.r), tex3, alphaBlend.g), tex4, alphaBlend.b);
}

void calcADTHeightFragMaterial(
    in vec2 tcLayer0, in vec2 tcLayer1, in vec2 tcLayer2, in vec2 tcLayer3,
    in sampler2D t_Layer0, in sampler2D t_Layer1, in sampler2D t_Layer2, in sampler2D t_Layer3,
    in sampler2D t_LayerHeight0, in sampler2D t_LayerHeight1, in sampler2D t_LayerHeight2, in sampler2D t_LayerHeight3,
    in vec2 tcAlpha,
    in sampler2D t_alphaTex,
    vec4 heightOffset,
    vec4 heightScale,

    out vec4 matDiffuse
) {
    vec3 alphaBlend = texture( t_alphaTex, tcAlpha).rgb;

    float minusAlphaBlendSum = (1.0 - clamp(dot(alphaBlend, vec3(1.0)), 0.0, 1.0));
    vec4 weightsVector = vec4(minusAlphaBlendSum, alphaBlend);
    float weightedTexture_x = (minusAlphaBlendSum * ((texture(t_LayerHeight0, tcLayer0).w * heightScale[0]) + heightOffset[0]));
    float weightedTexture_y = (weightsVector.y * ((texture(t_LayerHeight1, tcLayer1).w * heightScale[1]) + heightOffset[1]));
    float weightedTexture_z = (weightsVector.z * ((texture(t_LayerHeight2, tcLayer2).w * heightScale[2]) + heightOffset[2]));
    float weightedTexture_w = (weightsVector.w * ((texture(t_LayerHeight3, tcLayer3).w * heightScale[3]) + heightOffset[3]));
    vec4 weights = vec4(weightedTexture_x, weightedTexture_y, weightedTexture_z, weightedTexture_w);
    vec4 weights_temp = (weights * (vec4(1.0) - clamp((vec4(max(max(weightedTexture_x, weightedTexture_y), max(weightedTexture_z, weightedTexture_w))) - weights), 0.0, 1.0)));
    vec4 weightsNormalized = (weights_temp / vec4(dot(vec4(1.0), weights_temp)));

    vec4 weightedLayer_0 = (texture(t_Layer0, tcLayer0) * weightsNormalized.x);
    vec3 matDiffuse_0 = weightedLayer_0.xyz;
    float specBlend_0 = weightedLayer_0.w;

    vec4 weightedLayer_1 = (texture(t_Layer1, tcLayer1) * weightsNormalized.y);
    vec3 matDiffuse_1 = (matDiffuse_0 + weightedLayer_1.xyz);
    float specBlend_1 = (specBlend_0 + weightedLayer_1.w);

    vec4 weightedLayer_2 = (texture(t_Layer2, tcLayer2) * weightsNormalized.z);
    vec3 matDiffuse_2 = (matDiffuse_1 + weightedLayer_2.xyz);
    float specBlend_2 = (specBlend_1 + weightedLayer_2.w);

    vec4 weightedLayer_3 = (texture(t_Layer3, tcLayer3) * weightsNormalized.w);
    vec3 matDiffuse_3 = (matDiffuse_2 + weightedLayer_3.xyz);
    float specBlend_3 = (specBlend_2 + weightedLayer_3.w);

    matDiffuse = vec4(matDiffuse_3, specBlend_3);
}

#endif