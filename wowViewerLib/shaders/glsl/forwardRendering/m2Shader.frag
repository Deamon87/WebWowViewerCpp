#version 450

#extension GL_GOOGLE_include_directive: require

#ifndef MAX_MATRIX_NUM
#define MAX_MATRIX_NUM 220
#endif

precision highp float;
precision highp int;

#include "../common/commonLightFunctions.glsl"
#include "../common/commonFogFunctions.glsl"
#include "../common/commonM2Material.glsl"

struct LocalLight
{
    vec4 color;
    vec4 position;
    vec4 attenuation;
};

layout(location=0) in vec2 vTexCoord;
layout(location=1) in vec2 vTexCoord2;
layout(location=2) in vec2 vTexCoord3;
layout(location=3) in vec3 vNormal;
layout(location=4) in vec3 vPosition;
layout(location=5) in vec4 vMeshColorAlpha;

layout(location=0) out vec4 outputColor;

layout(std140, set=0, binding=0) uniform sceneWideBlockVSPS {
    SceneWideParams scene;
    PSFog fogData;
};

layout(std140, set=0, binding=1) uniform modelWideBlockVS {
    mat4 uPlacementMat;
    mat4 uBoneMatrixes[MAX_MATRIX_NUM];
};

//Whole model
layout(std140, set=0, binding=3) uniform modelWideBlockPS {
    InteriorLightParam intLight;
    LocalLight pc_lights[4];
    ivec4 lightCountAndBcHack;
    vec4 interiorExteriorBlend;
};

//Individual meshes
layout(std140, set=0, binding=4) uniform meshWideBlockPS {
    ivec4 PixelShader_UnFogged_IsAffectedByLight_blendMode;
    vec4 uTexSampleAlpha;
};

layout(set=1,binding=5) uniform sampler2D uTexture;
layout(set=1,binding=6) uniform sampler2D uTexture2;
layout(set=1,binding=7) uniform sampler2D uTexture3;
layout(set=1,binding=8) uniform sampler2D uTexture4;

void main() {
    /* Animation support */
    vec2 texCoord = vTexCoord.xy;
    vec2 texCoord2 = vTexCoord2.xy;
    vec2 texCoord3 = vTexCoord3.xy;

    vec4 finalColor = vec4(0);
    vec3 meshResColor = vMeshColorAlpha.rgb;

    vec3 accumLight = vec3(0.0);
    if ((PixelShader_UnFogged_IsAffectedByLight_blendMode.z == 1)) {
        vec3 vPos3 = vPosition.xyz;
        vec3 vNormal3 = normalize(vNormal.xyz);
        vec3 lightColor = vec3(0.0);
        int count = int(pc_lights[0].attenuation.w);

        for (int index = 0; index < 4; index++)
        {
            if (index >= lightCountAndBcHack.x) break;

            LocalLight lightRecord = pc_lights[index];
            vec3 vectorToLight = ((scene.uLookAtMat * (uPlacementMat * lightRecord.position)).xyz - vPos3);
            float distanceToLightSqr = dot(vectorToLight, vectorToLight);
            float distanceToLightInv = inversesqrt(distanceToLightSqr);
            float distanceToLight = (distanceToLightSqr * distanceToLightInv);
            float diffuseTerm1 = max((dot(vectorToLight, vNormal3) * distanceToLightInv), 0.0);
            vec4 attenuationRec = lightRecord.attenuation;

            float attenuation = (1.0 - clamp((distanceToLight - attenuationRec.x) * (1.0 / (attenuationRec.z - attenuationRec.x)), 0.0, 1.0));

            vec3 attenuatedColor = attenuation * lightRecord.color.xyz;
            lightColor = (lightColor + vec3(attenuatedColor * attenuatedColor * diffuseTerm1 ));
        }

        meshResColor.rgb = clamp(lightColor , 0.0, 1.0);
        accumLight = mix(lightColor.rgb, meshResColor.rgb, lightCountAndBcHack.y);
        //finalColor.rgb =  finalColor.rgb * lightColor;
    }

    float finalOpacity = 0.0;
    vec3 matDiffuse;
    vec3 specular;

    int uPixelShader = PixelShader_UnFogged_IsAffectedByLight_blendMode.x;
    int blendMode = PixelShader_UnFogged_IsAffectedByLight_blendMode.w;

    bool doDiscard = false;

    calcM2FragMaterial(uPixelShader,
        uTexture, uTexture2, uTexture3, uTexture4,
        texCoord, texCoord2, texCoord3,
        vMeshColorAlpha.rgb, vMeshColorAlpha.a,
        uTexSampleAlpha.rgb,
        blendMode,
        matDiffuse, specular, finalOpacity, doDiscard
    );

    if (doDiscard)
        discard;

    finalColor = vec4(
        calcLight(
            matDiffuse,
            vNormal,
            PixelShader_UnFogged_IsAffectedByLight_blendMode.z > 0,
            interiorExteriorBlend.x,
            scene,
            intLight,
            accumLight,
            vec3(0.0),
            specular,
            vec3(0.0)
        ) ,
        finalOpacity
    );

    outputColor = finalColor;

    int uUnFogged = PixelShader_UnFogged_IsAffectedByLight_blendMode.y;
    if (uUnFogged == 0) {
        vec3 sunDir =
            mix(
                scene.uInteriorSunDir,
                scene.extLight.uExteriorDirectColorDir,
                interiorExteriorBlend.x
            )
            .xyz;

        finalColor = makeFog(fogData, finalColor, vPosition.xyz, sunDir.xyz, PixelShader_UnFogged_IsAffectedByLight_blendMode.w);
    }
//    finalColor.rgb = finalColor.rgb;


    //Forward rendering without lights
    outputColor = finalColor;

    //Deferred rendering
    //gl_FragColor = finalColor;
//    gl_FragData[0] = vec4(vec3(fs_Depth), 1.0);
//    gl_FragData[1] = vec4(vPosition.xyz,0);
//    gl_FragData[2] = vec4(vNormal.xyz,0);
//    gl_FragData[3] = finalColor;

}
