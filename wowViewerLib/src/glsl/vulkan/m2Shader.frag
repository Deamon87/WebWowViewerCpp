#version 450

#extension GL_GOOGLE_include_directive: require

precision highp float;

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
layout(location=5) in vec4 vDiffuseColor;

layout(location=0) out vec4 outputColor;

layout(std140, set=0, binding=0) uniform sceneWideBlockVSPS {
    mat4 uLookAtMat;
    mat4 uPMatrix;
};

//Whole model
layout(std140, set=0, binding=3) uniform modelWideBlockPS {
    vec4 uViewUp;
    vec4 uSunDirAndFogStart;
    vec4 uSunColorAndFogEnd;
    vec4 uAmbientLight;
};

//Individual meshes
layout(std140, set=0, binding=4) uniform meshWideBlockPS {
    ivec4 PixelShader_UnFogged_IsAffectedByLight_LightCount;
    vec4 uFogColorAndAlphaTest;
    LocalLight pc_lights[4];
    vec4 uPcColor;
};

//layout(binding=5) uniform sampler2D uTexture;
//layout(binding=6) uniform sampler2D uTexture2;
//layout(binding=7) uniform sampler2D uTexture3;
//layout(binding=8) uniform sampler2D uTexture4;


vec3 makeDiffTerm(vec3 matDiffuse, vec3 accumLight) {
//    return matDiffuse;
    vec3 currColor;
    float mult = 1.0;
    vec3 lDiffuse = vec3(0.0, 0.0, 0.0);
    if (PixelShader_UnFogged_IsAffectedByLight_LightCount.z == 1) {
        vec3 normalizedN = normalize(vNormal);
        float nDotL = clamp(dot(normalizedN, -(uSunDirAndFogStart.xyz)), 0.0, 1.0);
        float nDotUp = dot(normalizedN, uViewUp.xyz);

        vec4 AmbientLight = uAmbientLight;

        vec3 adjAmbient = (AmbientLight.rgb );
        vec3 adjHorizAmbient = (AmbientLight.rgb );
        vec3 adjGroundAmbient = (AmbientLight.rgb );

        if ((nDotUp >= 0.0))
        {
            currColor = mix(adjHorizAmbient, adjAmbient, vec3(nDotUp));
        }
        else
        {
            currColor= mix(adjHorizAmbient, adjGroundAmbient, vec3(-(nDotUp)));
        }

        vec3 skyColor = (currColor * 1.10000002);
        vec3 groundColor = (currColor* 0.699999988);


        lDiffuse = (uSunColorAndFogEnd.xyz * nDotL);
        currColor = mix(groundColor, skyColor, vec3((0.5 + (0.5 * nDotL))));
//

    } else {
        currColor = vec3 (1.0, 1.0, 1.0) ;
        accumLight = vec3(0,0,0);
        mult = 1.0;
    }


//    return currColor.rgb * matDiffuse;
//    return sqrt((matDiffuse*matDiffuse)*0.5 + currColor.rgb*(matDiffuse*matDiffuse));
    vec3 gammaDiffTerm = matDiffuse * (currColor + lDiffuse);
    vec3 linearDiffTerm = (matDiffuse * matDiffuse) * accumLight;
//    return sqrt((matDiffuse*matDiffuse)*mult + currColor.rgb*(matDiffuse*matDiffuse)) ;
    return sqrt(gammaDiffTerm*gammaDiffTerm + linearDiffTerm) ;
}

#include "../common/commonFunctions.glsl"

void main() {
    /* Animation support */
    vec2 texCoord = vTexCoord.xy;
    vec2 texCoord2 = vTexCoord2.xy;
    vec2 texCoord3 = vTexCoord3.xy;

    /* Get color from texture */
//    vec4 tex = texture(uTexture, texCoord).rgba;
//    vec4 tex2 = texture(uTexture2, texCoord2).rgba;
//    vec4 tex3 = texture(uTexture3, texCoord3).rgba;
//
//    vec4 tex2WithTextCoord1 = texture(uTexture2,texCoord);
//    vec4 tex3WithTextCoord1 = texture(uTexture3,texCoord);
//    vec4 tex4WithTextCoord2 = texture(uTexture4,texCoord2);

    vec4 finalColor = vec4(0);
    vec4 meshResColor = vDiffuseColor;

//    if(meshResColor.a < uAlphaTest)
//        discard;
    vec3 accumLight;
    if ((PixelShader_UnFogged_IsAffectedByLight_LightCount.z == 1)) {
        vec3 vPos3 = vPosition.xyz;
        vec3 vNormal3 = normalize(vNormal.xyz);
        vec3 lightColor = vec3(0.0);
        int count = int(pc_lights[0].attenuation.w);
        int index = 0;
        for (;;)
        {
            if ( index >= PixelShader_UnFogged_IsAffectedByLight_LightCount.w) break;
            LocalLight lightRecord = pc_lights[index];
            vec3 vectorToLight = ((lightRecord.position).xyz - vPos3);
            float distanceToLightSqr = dot(vectorToLight, vectorToLight);
            float distanceToLightInv = inversesqrt(distanceToLightSqr);
            float distanceToLight = (distanceToLightSqr * distanceToLightInv);
            float diffuseTerm1 = max((dot(vectorToLight, vNormal3) * distanceToLightInv), 0.0);
            vec4 attenuationRec = lightRecord.attenuation;

            float attenuation = (1.0 - clamp((distanceToLight - attenuationRec.x) * (1.0 / (attenuationRec.z - attenuationRec.x)), 0.0, 1.0));

            vec3 attenuatedColor = attenuation * lightRecord.color.xyz * attenuationRec.y;
            lightColor = (lightColor + vec3(attenuatedColor * attenuatedColor * diffuseTerm1 ));
            index++;
        }
        meshResColor.rgb = clamp(lightColor , 0.0, 1.0);
        accumLight = meshResColor.rgb;
        //finalColor.rgb =  finalColor.rgb * lightColor;
    }

    float opacity;
    float finalOpacity = 0.0;
    vec3 matDiffuse;
    vec3 specular = vec3(0.0, 0.0, 0.0);
    vec3 visParams = vec3(1.0, 1.0, 1.0);
    vec4 genericParams[3];
    genericParams[0] = vec4( 1.0, 1.0, 1.0, 1.0 );
    genericParams[1] = vec4( 1.0, 1.0, 1.0, 1.0 );
    genericParams[2] = vec4( 1.0, 1.0, 1.0, 1.0 );

    int uPixelShader = PixelShader_UnFogged_IsAffectedByLight_LightCount.x;


        matDiffuse = vDiffuseColor.rgb * 2.000000;
        opacity = vDiffuseColor.a;
        finalOpacity = opacity * visParams.r;


    finalColor = vec4(makeDiffTerm(matDiffuse, accumLight) + specular, finalOpacity);

    if(finalColor.a < uFogColorAndAlphaTest.w)
        discard;

    /*
    int uUnFogged = PixelShader_UnFogged_IsAffectedByLight_LightCount.y;
    float uFogEnd = uSunColorAndFogEnd.z;
    if (uUnFogged == 0) {
        vec3 fogColor = uFogColorAndAlphaTest.xyz;
        float fog_rate = 1.5;
        float fog_bias = 0.01;

        //vec4 fogHeightPlane = pc_fog.heightPlane;
        //float heightRate = pc_fog.color_and_heightRate.w;

        float distanceToCamera = length(vPosition.xyz);
        float z_depth = (distanceToCamera - fog_bias);
        float expFog = 1.0 / (exp((max(0.0, (z_depth - uSunDirAndFogStart.z)) * fog_rate)));
        //float height = (dot(fogHeightPlane.xyz, vPosition.xyz) + fogHeightPlane.w);
        //float heightFog = clamp((height * heightRate), 0, 1);
        float heightFog = 1.0;
        expFog = (expFog + heightFog);
        float endFadeFog = clamp(((uFogEnd - distanceToCamera) / (0.699999988 * uFogEnd)), 0.0, 1.0);
        float fog_out = min(expFog, endFadeFog);
        finalColor.rgba = vec4(mix(fogColor.rgb, finalColor.rgb, vec3(fog_out)), finalColor.a);
    }*/
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
