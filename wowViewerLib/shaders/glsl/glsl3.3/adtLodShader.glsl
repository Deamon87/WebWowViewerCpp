#ifdef COMPILING_VS
/* vertex shader code */
attribute float aHeight;
attribute float aIndex;

uniform vec3 uPos;
uniform mat4 uLookAtMat;
uniform mat4 uPMatrix;

varying vec2 vChunkCoords;
varying vec3 vPosition;

const float UNITSIZE_X =  (1600.0 / 3.0) / 128.0;
const float UNITSIZE_Y =  (1600.0 / 3.0) / 128.0;
const float ADTLOD_UNITSIZE = (1600.0 / 3.0) / 128.0;

void main() {

    float stepX = floor(aIndex / (129.0*129.0));
    float division = 129.0;
    if (stepX > 0.1)
        division = 128.0;
    float offset =stepX*129.0*129.0;

    float iX = mod(aIndex - offset, division) + stepX*0.5;
    float iY = floor((aIndex - offset)/division) + stepX*0.5;

//    if (stepX < 0.1 && iY > 126) {
//        iY  = 0;
//    }

    vec4 worldPoint = vec4(
        uPos.x - iY * UNITSIZE_Y,
        uPos.y - iX * UNITSIZE_X,
        aHeight,
        1);


//
//    float iX = mod(aIndex, 256.0);
//    float iY = floor(aIndex/256.0);
//
//    if (iX > 128.01) {
//        iY = iY + 0.5;
//        iX = iX - 128.5;
//    }
//
//    vec4 worldPoint = vec4(
//        uPos.x - iX * UNITSIZE_Y,
//        uPos.y - iY * UNITSIZE_X,
//        uPos.z + aHeight,
//        1);

    vChunkCoords = vec2(iX / 128.0, iY / 128.0);

    vPosition = (uLookAtMat * worldPoint).xyz;
    gl_Position = uPMatrix * uLookAtMat * worldPoint;
}
#endif //COMPILING_VS


#ifdef COMPILING_FS
precision highp float;

varying vec2 vChunkCoords;
varying vec3 vPosition;

uniform int uNewFormula;

uniform sampler2D uDiffuseTexture;
uniform sampler2D uNormalTexture;

uniform vec3 uViewUp;
uniform vec3 uSunDir;
uniform vec3 uSunColor;
uniform vec4 uAmbientLight;

uniform vec3 uFogColor;

uniform float uFogStart;
uniform float uFogEnd;

vec3 makeDiffTerm(vec3 matDiffuse, vec3 vNormal) {
  vec3 currColor;
    float mult = 1.0;
    vec3 lDiffuse = vec3(0.0, 0.0, 0.0);
    if (true) {
        vec3 normalizedN = normalize(vNormal);
        float nDotL = dot(normalizedN, -(uSunDir.xyz));
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

        lDiffuse = (uSunColor * clamp(nDotL, 0.0, 1.0));
        currColor = mix(groundColor, skyColor, vec3((0.5 + (0.5 * nDotL))));
    } else {
        currColor = vec3 (1.0, 1.0, 1.0) ;
        mult = 1.0;
    }

    vec3 gammaDiffTerm = matDiffuse * (currColor + lDiffuse);
//    vec3 linearDiffTerm = (matDiffuse * matDiffuse);
    vec3 linearDiffTerm = vec3(0.0, 0.0, 0.0);
    return sqrt(gammaDiffTerm*gammaDiffTerm + linearDiffTerm) ;
}


void main() {
    vec2 TextureCoords = vec2(vChunkCoords.x, vChunkCoords.y );


    vec3 matDiffuse;
    vec4 finalColor;

    vec4 texDiffuse =  texture2D(uDiffuseTexture, TextureCoords).rgba;
    matDiffuse.rgb = texDiffuse.rgb;
    vec3 vNormal = 2.0*texture2D(uNormalTexture, TextureCoords).rgb - 1.0 ;
    vNormal = vec3(-(vNormal.z), -(vNormal.x), vNormal.y);



    finalColor.rgba = vec4(makeDiffTerm(matDiffuse, vNormal), 1.0);

    //Spec part
    float specBlend = texDiffuse.a;
    vec3 halfVec = -(normalize((uSunDir.xyz + normalize(vPosition))));
    vec3 lSpecular = ((uSunColor * pow(max(0.0, dot(halfVec, vNormal)), 20.0)));
    vec3 specTerm = (vec3(specBlend) * lSpecular);
    finalColor.rgb += specTerm;

    // --- Fog start ---
    vec3 fogColor = uFogColor;
    float fog_start = uFogStart;
    float fog_end = uFogEnd;
    float fog_rate = 1.5;
    float fog_bias = 0.01;

    //vec4 fogHeightPlane = pc_fog.heightPlane;
    //float heightRate = pc_fog.color_and_heightRate.w;

    float distanceToCamera = length(vPosition.xyz);
    float z_depth = (distanceToCamera - fog_bias);
    float expFog = 1.0 / (exp((max(0.0, (z_depth - fog_start)) * fog_rate)));
    //float height = (dot(fogHeightPlane.xyz, vPosition.xyz) + fogHeightPlane.w);
    //float heightFog = clamp((height * heightRate), 0, 1);
    float heightFog = 1.0;
    expFog = (expFog + heightFog);
    float endFadeFog = clamp(((fog_end - distanceToCamera) / (0.699999988 * fog_end)), 0.0, 1.0);

    finalColor.rgb = mix(fogColor.rgb, finalColor.rgb, vec3(min(expFog, endFadeFog)));
    // --- Fog end ---

    finalColor.a = 1.0;
    gl_FragColor = finalColor;
}

#endif //COMPILING_FS