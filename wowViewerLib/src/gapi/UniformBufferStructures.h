//
// Created by deamon on 09.07.18.
//

#ifndef AWEBWOWVIEWERCPP_UNIFORMBUFFERSTRUCTURES_H
#define AWEBWOWVIEWERCPP_UNIFORMBUFFERSTRUCTURES_H

//Uniform buffer structures
#include <mathfu/glsl_mappings.h>
#include "../../3rdparty/mathfu/include/mathfu/glsl_mappings.h"

#define MAX_MATRIX_NUM 64

struct sceneWideBlockVSPS {
    mathfu::mat4 uLookAtMat;
    mathfu::mat4 uPMatrix;
};


//M2 Vertex buffer formats
struct modelWideBlockVS {
    mathfu::mat4 uPlacementMat;
    mathfu::mat4 uBoneMatrixes[MAX_MATRIX_NUM];
};

struct meshWideBlockVS {
    int VertexShader;
    int IsAffectedByLight;
    int padding[2];
    mathfu::vec4_packed Color_Transparency;

    mathfu::mat4 uTextMat[2];
};

//M2 Pixel buffer formats
struct modelWideBlockPS {
    mathfu::vec4_packed uViewUp;
    mathfu::vec4_packed uSunDirAndFogStart;
    mathfu::vec4_packed uSunColorAndFogEnd;
    mathfu::vec4_packed uAmbientLight;
};

struct LocalLight
{
    mathfu::vec4_packed color;
    mathfu::vec4_packed position;
    mathfu::vec4_packed attenuation;
};

struct meshWideBlockPS {
    int PixelShader;
    int UnFogged;
    int IsAffectedByLight;
    int LightCount;
    mathfu::vec4_packed uFogColorAndAlphaTest;
    LocalLight pc_lights[4];
    mathfu::vec4_packed uPcColor;
};

//M2 Pixel buffer format

#endif //AWEBWOWVIEWERCPP_UNIFORMBUFFERSTRUCTURES_H
