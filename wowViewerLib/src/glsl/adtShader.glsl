#ifdef COMPILING_VS
/* vertex shader code */
attribute float aHeight;
attribute vec4 aColor;
attribute vec4 aVertexLighting;
attribute vec3 aNormal;
attribute float aIndex;

uniform vec3 uPos;
uniform mat4 uLookAtMat;
uniform mat4 uPMatrix;

varying vec2 vChunkCoords;
varying vec3 vPosition;
varying vec4 vColor;
varying vec3 vNormal;
varying vec3 vVertexLighting;

const float UNITSIZE_X =  (1600.0 / 3.0) / 16.0 / 8.0;
const float UNITSIZE_Y =  (1600.0 / 3.0) / 16.0 / 8.0;

void main() {

/*
     Y
  X  0    1    2    3    4    5    6    7    8
        9   10   11   12   13   14   15   16
*/
    float iX = mod(aIndex, 17.0);
    float iY = floor(aIndex/17.0);

    if (iX > 8.01) {
        iY = iY + 0.5;
        iX = iX - 8.5;
    }

    vec4 worldPoint = vec4(
        uPos.x - iY * UNITSIZE_Y,
        uPos.y - iX * UNITSIZE_X,
        uPos.z + aHeight,
        1);

    vChunkCoords = vec2(iX, iY);

    vPosition = (uLookAtMat * worldPoint).xyz;
    vColor = aColor;
    vVertexLighting = aVertexLighting.rgb;
    vNormal = (uLookAtMat * vec4(aNormal, 0.0)).xyz;

    gl_Position = uPMatrix * uLookAtMat * worldPoint;
}
#endif //COMPILING_VS

#ifdef COMPILING_FS
precision highp float;

varying vec2 vChunkCoords;
varying vec3 vPosition;
varying vec4 vColor;
varying vec3 vNormal;
varying vec3 vVertexLighting;

uniform int uNewFormula;

uniform sampler2D uLayer0;
uniform sampler2D uLayer1;
uniform sampler2D uLayer2;
uniform sampler2D uLayer3;
uniform sampler2D uAlphaTexture;
uniform sampler2D uLayerHeight0;
uniform sampler2D uLayerHeight1;
uniform sampler2D uLayerHeight2;
uniform sampler2D uLayerHeight3;

uniform float uHeightScale[4];
uniform float uHeightOffset[4];

uniform vec3 uViewUp;
uniform vec3 uSunDir;
uniform vec3 uSunColor;
uniform vec4 uAmbientLight;

uniform vec3 uFogColor;

uniform float uFogStart;
uniform float uFogEnd;

/* Bicubic interpolation implementation */
/* Taken from http://www.codeproject.com/Articles/236394/Bi-Cubic-and-Bi-Linear-Interpolation-with-GLSL#Triangular */

float Triangular( float f ) {
	f = f / 2.0;
	if( f < 0.0 )
	{
		return ( f + 1.0 );
	}
	else
	{
		return ( 1.0 - f );
	}
	return 0.0;
}

float CatMullRom( float x ) {
    const float B = 0.0;
    const float C = 0.5;
    float f = x;
    if( f < 0.0 )
    {
        f = -f;
    }
    if( f < 1.0 )
    {
        return ( ( 12.0 - 9.0 * B - 6.0 * C ) * ( f * f * f ) +
            ( -18.0 + 12.0 * B + 6.0 * C ) * ( f * f ) +
            ( 6.0 - 2.0 * B ) ) / 6.0;
    }
    else if( f >= 1.0 && f < 2.0 )
    {
        return ( ( -B - 6.0 * C ) * ( f * f * f )
            + ( 6.0 * B + 30.0 * C ) * ( f *f ) +
            ( - ( 12.0 * B ) - 48.0 * C  ) * f +
            8.0 * B + 24.0 * C)/ 6.0;
    }
    else
    {
        return 0.0;
    }
}
vec4 BiCubic( sampler2D textureSampler, vec2 TexCoord, float widthMinClamp, float widthMaxClamp) {
    float fWidth = 256.0;
    float fHeight = 64.0;

    float texelSizeX = 1.0 / fWidth; //size of one texel
    float texelSizeY = 1.0 / fHeight; //size of one texel
    vec4 nSum = vec4( 0.0, 0.0, 0.0, 0.0 );
    vec4 nDenom = vec4( 0.0, 0.0, 0.0, 0.0 );
    float a = fract( TexCoord.x * fWidth ); // get the decimal part
    float b = fract( TexCoord.y * fHeight ); // get the decimal part
    for( int m = -1; m <=2; m++ )
    {
        for( int n =-1; n<= 2; n++)
        {
            vec2 coords = TexCoord + vec2( texelSizeX * float( m ), texelSizeY * float( n ));

            float xCoord = coords.x;
            xCoord = max(widthMinClamp, xCoord);
            xCoord = min(widthMaxClamp, xCoord);
            coords.x = xCoord;

            vec4 vecData = texture2D( textureSampler, coords);
            float f  = Triangular( float( m ) - a );
            vec4 vecCooef1 = vec4( f,f,f,f );
            float f1 = Triangular ( -( float( n ) - b ) );
            vec4 vecCoeef2 = vec4( f1, f1, f1, f1 );
            nSum = nSum + ( vecData * vecCoeef2 * vecCooef1  );
            nDenom = nDenom + (( vecCoeef2 * vecCooef1 ));
        }
    }
    return nSum / nDenom;
}
/********************************/

float filterFunc(float x) {
  //x = ceil(x * 16.0)/16.0;

  /*const float multiplier = 1.0;
  const float additive =  0.0;
  const float maxThreshold = 0.0;
  const float minThreshold = 1.0;


  x = x * multiplier + additive;
  x = max(x, maxThreshold);
  x = min(x, minThreshold);
  return x;*/
    return x;
}

vec3 makeDiffTerm(vec3 matDiffuse) {
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
    vec3 linearDiffTerm = (matDiffuse * matDiffuse) * vVertexLighting;
    return sqrt(gammaDiffTerm*gammaDiffTerm + linearDiffTerm) ;
}


void main() {
    vec2 vTexCoord = vChunkCoords;
    const float threshold = 1.5;

    vec2 alphaCoord = vec2(vChunkCoords.x/8.0, vChunkCoords.y/8.0 );
    vec3 alpha = texture2D( uAlphaTexture, alphaCoord).gba;

   // then layer 0 is 1-sum to fill up
   vec4 layer_weights = vec4(1.0 - clamp(dot(alpha, vec3(1.0)), 0.0, 1.0), alpha);

    vec4 tex4 = texture2D(uLayer3, vTexCoord).rgba;
    vec4 tex3 = texture2D(uLayer2, vTexCoord).rgba;
    vec4 tex2 = texture2D(uLayer1, vTexCoord).rgba;
    vec4 tex1 = texture2D(uLayer0, vTexCoord).rgba;

    vec4 layer_pct = vec4 ( layer_weights.x * (texture2D(uLayerHeight0, alphaCoord).a * uHeightScale[0] + uHeightOffset[0])
                         , layer_weights.y * (texture2D(uLayerHeight1, alphaCoord).a * uHeightScale[1] + uHeightOffset[1])
                         , layer_weights.z * (texture2D(uLayerHeight2, alphaCoord).a * uHeightScale[2] + uHeightOffset[2])
                         , layer_weights.w * (texture2D(uLayerHeight3, alphaCoord).a * uHeightScale[3] + uHeightOffset[3])
                         );


    vec4 layer_pct_max = vec4(max(max(max(layer_pct.x, layer_pct.y), layer_pct.z), layer_pct.w));
    layer_pct = layer_pct * (vec4(1.0) - clamp(layer_pct_max - layer_pct, 0.0, 1.0));
    layer_pct = layer_pct / vec4(dot(layer_pct, vec4(1.0)));

    vec4 weightedLayer_0 = tex1 * layer_pct.x;
    vec4 weightedLayer_1 = tex2 * layer_pct.y;
    vec4 weightedLayer_2 = tex3 * layer_pct.z;
    vec4 weightedLayer_3 = tex4 * layer_pct.w;

    vec4 finalColor;
    vec3 matDiffuse = (weightedLayer_0.rgb + weightedLayer_1.rgb + weightedLayer_2.rgb + weightedLayer_3.rgb);
    float specBlend = (weightedLayer_0.a + weightedLayer_1.a + weightedLayer_2.a + weightedLayer_3.a);

    matDiffuse.rgb = matDiffuse.rgb * 2.0 * vColor.rgb;
    finalColor.rgba = vec4(makeDiffTerm(matDiffuse), 1.0);

    //Spec part
//    float specBlend = tex1.a;
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