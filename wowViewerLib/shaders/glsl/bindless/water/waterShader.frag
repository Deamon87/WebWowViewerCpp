#version 450

#extension GL_GOOGLE_include_directive: require
#extension GL_EXT_nonuniform_qualifier: require

#include "../../common/commonLightFunctions.glsl"
#include "../../common/commonFogFunctions.glsl"

precision highp float;
precision highp int;

layout(location=0) in vec3 vPosition;
layout(location=1) in vec2 vTextCoords;
layout(location=2) in vec3 vNormal;
layout(location=3) in flat int meshInd;

#include "../../common/commonUboSceneData.glsl"
#include "../../common/commonWaterIndirect.glsl"

layout(location = 0) out vec4 outColor;

const InteriorLightParam intLight = {
    vec4(0,0,0,0),
    vec4(0,0,0,0),
    vec4(0,0,0,0),
    vec4(0,0,0,1),
    vec4(0,0,0,0)
};

const float ROUNDING_ERROR_f32 = 0.001f;
bool feq(const float a, const float b)
{
    return (a + ROUNDING_ERROR_f32 >= b) && (a - ROUNDING_ERROR_f32 <= b);
}

vec2 GetTexScrollMtx(float time, vec2 scrollVec) {
    float scrollX = 0.0f;
    float scrollY = 0.0f;

    if (!feq(scrollVec.x, 0.0)) {
        scrollX = (int(time) % int(1000.0f / scrollVec.x)) / float(int(1000.0f / scrollVec.x));
    }
    if (!feq(scrollVec.y, 0.0)) {
        scrollY = (int(time) % int(1000.0f / scrollVec.y)) / float(int(1000.0f / scrollVec.y));
    }

    return vec2(scrollX, scrollY);
}

#define M_PI 3.1415926538
mat4 rotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;

    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
    oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
    oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
    0.0,                                0.0,                                0.0,                                1.0);
}

mat4 rotationZ(float angle) {
    return mat4(
        cos(angle), -sin(angle), 0.0,  0.0 ,
        sin(angle), cos(angle),  0.0,  0.0 ,
        0.0,        0.0,         1.0,  0.0 ,
        0.0,        0.0,         0.0,  1.0
    );
}

void main() {
    WaterBindless waterBindless = waterBindlesses[meshInd];
    WaterData waterData = waterDatas[waterBindless.waterDataInd_placementMatInd_textureInd.x];

// MaterialId:
//    1,3 - Water
//    2,4 - Magma
//    5 - Mercury,
//    10 - Fog
//    12 - LeyLine
//    13 - Fel
//    14 - Swamp
//    18 - Azerithe
//    8 - is probably debug material called Grid

    vec4 color = vec4(0.0);
    int liquidFlags = waterData.materialId_liquidFlags.y;
    int materialId = waterData.materialId_liquidFlags.x;

    if ((liquidFlags & 1024) > 0) {// Ocean
        color = scene.closeOceanColor_shallowAlpha;
    } else if (liquidFlags == 15) { //River/Lake
        //Query river color
        color = scene.closeRiverColor_shallowAlpha;
    } else {
        color = vec4(waterData.matColor.xyz, 0.7);
    }

    vec2 animatedUV = vTextCoords;

    float sceneTime = scene.uViewUpSceneTime.w;
    float float0 = waterData.float0_float1.x;
    float float1 = waterData.float0_float1.y;
    if (materialId == 1 || materialId == 3) {
        animatedUV *= float0;
//        animatedUV = (rotationMatrix(vec3(0,0,1), float1 * (M_PI / 180.0f)) * vec4(animatedUV, 0.0, 0.0)).xy;
        animatedUV = (rotationZ(float1 * (M_PI / 180.0f)) * vec4(animatedUV, 0.0, 0.0)).xy;
    } else if (materialId == 2 || materialId == 4) {
        animatedUV += GetTexScrollMtx(sceneTime, vec2(float0,float1));
    };

    vec3 matDiffuse = color.rgb+texture(s_Textures[waterBindless.waterDataInd_placementMatInd_textureInd.z], animatedUV).rgb;

    vec3 sunDir = scene.extLight.uExteriorDirectColorDir.xyz;


    vec4 finalColor = vec4(matDiffuse, 1.0);

    //Magma is not affected by light
    if (materialId != 2 && materialId != 4) {
        finalColor = vec4(
            calcLight(
                matDiffuse,
                vNormal,
                true,
                0,
                scene,
                intLight,
                vec3(0.0)  /* accumLight */,
                vec3(0.0),
                vec3(0.0), /* specular */
                vec3(0.0),
                1.0 /* ao */
            ),
            1.0
        );
    }

    //BlendMode is always GxBlend_Alpha
    finalColor.rgb = makeFog2(fogData/*, int(scene.extLight.adtSpecMult_fogCount.y)*/, finalColor, scene.uViewUpSceneTime.xyz, vPosition.xyz, sunDir.xyz, 2).rgb;

    outColor = vec4(finalColor.rgb, 0.7);
}
