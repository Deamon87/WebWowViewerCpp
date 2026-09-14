#ifndef COMMON_ADT_MATERIAL_SLANG
#define COMMON_ADT_MATERIAL_SLANG

import glsl;

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

const float TILESIZE = (1600.0 / 3.0);       //533.3333333333333
const float CHUNKSIZE = TILESIZE / 16.0;     //33.3333333333333
const float UNITSIZE =  CHUNKSIZE / 8.0;     //4.166666666666663

void calcAdtAlphaUV(in int indexInMCNK, in ivec2 globalChunkIndex, in vec3 globalPosOffset, out vec2 alphaCoords, out vec2 o_texCoord, out vec2 worldPointXY) {
    float iX = mod(indexInMCNK, 17.0);
    float iY = floor(indexInMCNK/17.0);

    if (iX > 8.01) {
        iY = iY + 0.5;
        iX = iX - 8.5;
    }

    vec2 adtMCNKPos = vec2(
        (512.0 - float(globalChunkIndex.y)) * CHUNKSIZE + globalPosOffset.x,
        (512.0 - float(globalChunkIndex.x)) * CHUNKSIZE + globalPosOffset.y
    );

    worldPointXY = vec2(
        adtMCNKPos.x - iY * UNITSIZE,
        adtMCNKPos.y - iX * UNITSIZE
    );

    // Top left point (17058 17063) becomes (0,0)
    vec2 coordsInAdtIndexSpace = 32.0f * TILESIZE - adtMCNKPos.yx;
    // Add half chunk to do little offset before doing floor in next step
    coordsInAdtIndexSpace += vec2(CHUNKSIZE/2.0);

    vec2 ADTIndex = floor(coordsInAdtIndexSpace / TILESIZE);
    vec2 fixedPos = (32.0f * TILESIZE) - (ADTIndex.yx*16) * CHUNKSIZE;

    vec2 chunkCoords = vec2(iX, iY);

    alphaCoords = ((vec2(32.0f) - ADTIndex) * TILESIZE - worldPointXY.yx) / TILESIZE;
    alphaCoords.x = fixADTUVBorder(alphaCoords.x, chunkCoords.x/8.0);
    alphaCoords.y = fixADTUVBorder(alphaCoords.y, chunkCoords.y/8.0);

    o_texCoord = -(worldPointXY - fixedPos).yx;
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

vec2 transformADTUV(in vec2 uv, in int layer, in float sceneTime, in ivec4 animation_rotationPerLayer,
                    in ivec4 animation_speedPerLayer, in vec4 scaleFactorPerLayer)
{
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

const int MAX_LAYER_COUNT = 8;

void calcADTOrigFragMaterial<let NUM_LAYERS : int>(
    in vec4 layer[MAX_LAYER_COUNT],
    in vec4 alpha[2],
    in bool useWeightedBlend,
    out vec4 matDiffuse
) {
    vec3 alphaBlend = alpha[0].gba;
    vec4 alphaBlend2 = alpha[1].rbga;

    if (!useWeightedBlend) {
        matDiffuse = mix(mix(mix(layer[0], layer[1], alphaBlend.r), layer[2], alphaBlend.g), layer[3], alphaBlend.b);
        if (NUM_LAYERS > 4) {
            matDiffuse += mix(mix(mix(mix(matDiffuse, layer[4], alphaBlend2.r), layer[5], alphaBlend2.g), layer[6], alphaBlend2.b), layer[7], alphaBlend2.a);
        }
    } else {
        float alphaSum = dot(alphaBlend, vec3(1.0));
        if (NUM_LAYERS > 4) {
            alphaSum += dot(alpha[1].rgba, vec4(1.0));
        }

        float minusAlphaBlendSum = (1.0 - clamp(alphaSum, 0.0, 1.0));
        float weights[MAX_LAYER_COUNT] = {
            minusAlphaBlendSum, alphaBlend.x, alphaBlend.y, alphaBlend.z,
            alphaBlend2.x, alphaBlend2.y, alphaBlend2.z, alphaBlend2.w
        };

        matDiffuse = vec4(0);
        for (int i = 0; i < NUM_LAYERS; i++) {
            matDiffuse += layer[i] * weights[i];
        }
    }
}

// Template function that works with different layer counts
void calcADTHeightFragMaterial<let NUM_LAYERS : int>(
    in vec4 layer[MAX_LAYER_COUNT],
    in vec4 layerHeight[MAX_LAYER_COUNT],
    in vec4 alpha[2],
    in float heightOffset[MAX_LAYER_COUNT],
    in float heightScale[MAX_LAYER_COUNT],
    out vec4 result
) {
    vec3 alphaBlend = alpha[0].gba;

    float alphaSum = dot(alphaBlend, vec3(1.0));
    if (NUM_LAYERS > 4) {
        alphaSum += dot(alpha[1].rgba, vec4(1.0));
    }

    float minusAlphaBlendSum = (1.0 - clamp(alphaSum, 0.0, 1.0));
    float weightsVector[NUM_LAYERS];
    weightsVector[0] = minusAlphaBlendSum;
    for (int i = 1; i < 4; i++) {
        weightsVector[i] = alphaBlend[i - 1];
    }
    if (NUM_LAYERS > 4) {
        for (int i = 0; i < NUM_LAYERS-4; i++) {
            weightsVector[i+4] = alpha[1][i];
        }
    }


    // Compute weighted heights
    float weights[NUM_LAYERS];
    float maxWeight = 0.0;
    for (int i = 0; i < NUM_LAYERS; i++) {
        weights[i] = weightsVector[i] * ((layerHeight[i].w * heightScale[i]) + heightOffset[i]);
        maxWeight = max(maxWeight, weights[i]);
    }

    // Normalize weights based on height blending
    float weights_temp[NUM_LAYERS];
    float weightsSum = 0.0;
    for (int i = 0; i < NUM_LAYERS; i++) {
        weights_temp[i] = weights[i] * (1.0 - clamp(maxWeight - weights[i], 0.0, 1.0));
        weightsSum += weights_temp[i];
    }

    float weightsNormalized[NUM_LAYERS];
    for (int i = 0; i < NUM_LAYERS; i++) {
        weightsNormalized[i] = weights_temp[i] / weightsSum;
    }

    // Blend all layers
    vec3 matDiffuse = vector<float, 3>(0.0);
    float specBlend = 0.0;
    for (int i = 0; i < NUM_LAYERS; i++) {
        vec4 weightedLayer = layer[i] * weightsNormalized[i];
        matDiffuse += weightedLayer.xyz;
        specBlend += weightedLayer.w;
    }

    result = vec4(matDiffuse, specBlend);
}


#endif