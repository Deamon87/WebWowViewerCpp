#ifndef LIQUID_MERCURY_MATERIAL_SLANG
#define LIQUID_MERCURY_MATERIAL_SLANG

import glsl;

#include "../commonLiquidStructs.slang"
#include "./commonLiquidFunctions.slang"
#include "../commonFogFunctions.slang"

vec4 calcMercuryLiquidMat(in LiquidInstanceData v0, in MercuryData v1, in SceneWideParams v2, in PSFog v3, in vec3 v4, in vec3 v5, in vec2 v6, in vec2 v7, in vec2 v8, in float v9, in sampler2D v10)
{
  float v11 = v2.uViewUpSceneTime.w;
  mat4 v12[4];
  mat4 v13[4];
  float v16 = v1.f0_f1_f2_f3.z;
  float v17 = v1.f0_f1_f2_f3.w;
  float v18 = v1.f4_f5_f6_f7.x;
  float v19 = v1.f4_f5_f6_f7.y;
  float v20 = v19 * 0.017453292f;
  float v21 = cos(v20);
  float v22 = sin(v20);
  v13[0] = mat4(vec4(v21 * v17, v22 * v17, 0.0, 0.0), vec4(-v22 * v17, v21 * v17, 0.0, 0.0), vec4(0.0, 0.0, v17, 0.0), vec4(0.0, 0.0, 0.0, 1.0));
  v13[1] = mat4(vec4(v21 * v18, v22 * v18, 0.0, 0.0), vec4(-v22 * v18, v21 * v18, 0.0, 0.0), vec4(0.0, 0.0, v18, 0.0), vec4(0.0, 0.0, 0.0, 1.0));
  v13[2] = mat4(vec4(1.0, 0.0, 0.0, 0.0), vec4(0.0, v16, 0.0, 0.0), vec4(0.0, 0.0, 1.0, 0.0), vec4(0.0, 0.0, 0.0, 1.0));
  vec2 v23 = GetTexScrollMtx(v11, vec2(0.01f, 0.0f));
  v13[3] = mat4(vec4(1.0, 0.0, 0.0, 0.0), vec4(0.0, 1.0, 0.0, 0.0), vec4(0.0, 0.0, 1.0, 0.0), vec4(v23.x, v23.y, 0.0, 1.0));
  v12[0] = v13[0];
  v12[1] = v13[1];
  v12[2] = v13[2];
  v12[3] = v13[3];
  mat4 v14[4];
  v14[0] = v12[0];
  v14[1] = v12[1];
  v14[2] = v12[2];
  v14[3] = v12[3];
  float v25 = v1.f4_f5_f6_f7.z;
  float v31 = v1.f12_f13_f14_f15.y;
  float v32 = v1.f12_f13_f14_f15.z;
  float v33 = v1.f12_f13_f14_f15.w;
  vec4 v34 = vec4(v31, v32, v33, v25);
  vec4 v37;
  v37.xy = (v14[0] * vec4(v7 / 32.0, 0.0, 1.0)).xy;
  v37.zw = (v14[1] * vec4(-(v7 + 0.5) / 32.0, 0.0, 1.0)).xy;
  vec4 v39;
  v39.xy = (v14[3] * vec4(16.0 * v37.xy, 0.0, 1.0)).yx;
  v39.zw = (v14[3] * vec4(32.0 * v37.zw, 0.0, 1.0)).yx;
  v37.xy = (v14[3] * vec4(v37.xy, 0.0, 1.0)).yx;
  v37.zw = (v14[3] * vec4(v37.zw, 0.0, 1.0)).yx;
  float v41 = (v9 == 0.0) ? 0.0 : 1.0;
  vec3 v42 = (v2.uInvLookAtMat * vec4(0.0, 0.0, 0.0, 1.0)).xyz;
  vec3 v43 = v4 - v42;
  float v44 = sqrt(v43.x * v43.x + v43.y * v43.y + v43.z * v43.z);
  vec3 v45 = v43 / v44;
  vec3 v46 = vec3(0.0, 0.0, 1.0);
  float v48 = abs(sceneViewDepth(v6, v2));
  float v49 = abs(v5.z);
  float v50 = v48 - v49;
  vec3 v52;
  float v57 = v34.w;
  vec2 v58 = texture(v10, v37.xy).xy + texture(v10, v37.zw).xy - 1.0;
  v58 = v58 * 2.0;
  vec2 v59 = texture(v10, v39.xy).xy + texture(v10, v39.zw).xy - 1.0;
  v59 = v59 * 2.0;
  vec3 v60 = vec3(0.7 * v59 + (v58 - 0.7 * v59) * min(max(0.2 + v44 / 1000.0, 0.0), 1.0), 1.0);
  v60.xy = v60.xy * v57;
  v52 = v60 / sqrt(v60.x * v60.x + v60.y * v60.y + v60.z * v60.z);
  v46.xy = v46.xy + v34.y * v52.xy;
  v46 = v46 / sqrt(v46.x * v46.x + v46.y * v46.y + v46.z * v46.z);
  float v63;
  vec3 v64 = -v45;
  float v66 = 1.0 - min(max(v64.x * v46.x + v64.y * v46.y + v64.z * v46.z, 0.0), 1.0);
  v63 = v66 * v66;
  float v68 = min(max((v50 - 0.0) / (1.2 * min(max(v63 * 0.5 + 0.5, 0.0), 1.0) - 0.0), 0.0f), 1.0f) * min(max((v50 - 0.0) / (1.2 * min(max(v63 * 0.5 + 0.5, 0.0), 1.0) - 0.0), 0.0f), 1.0f) * (3.0f - 2.0f * min(max((v50 - 0.0) / (1.2 * min(max(v63 * 0.5 + 0.5, 0.0), 1.0) - 0.0), 0.0f), 1.0f));
  float v69 = 1.0 - min(max((v50 * (0.0 + (1.0 - 0.0) * (1.0 - v45.z)) - 0.0) / (2.0 * min(max(v63 * 0.5 + 0.5, 0.0), 1.0) - 0.0), 0.0f), 1.0f) * min(max((v50 * (0.0 + (1.0 - 0.0) * (1.0 - v45.z)) - 0.0) / (2.0 * min(max(v63 * 0.5 + 0.5, 0.0), 1.0) - 0.0), 0.0f), 1.0f) * (3.0f - 2.0f * min(max((v50 * (0.0 + (1.0 - 0.0) * (1.0 - v45.z)) - 0.0) / (2.0 * min(max(v63 * 0.5 + 0.5, 0.0), 1.0) - 0.0), 0.0f), 1.0f));
  v46 = v46 + v69 * v45;
  v46 = v46 / sqrt(v46.x * v46.x + v46.y * v46.y + v46.z * v46.z);
  vec3 v70;
  vec3 v76 = reflect(v45, v46 * vec3(1.0, 1.0, reflectionParams.w) / sqrt((v46 * vec3(1.0, 1.0, reflectionParams.w)).x * (v46 * vec3(1.0, 1.0, reflectionParams.w)).x + (v46 * vec3(1.0, 1.0, reflectionParams.w)).y * (v46 * vec3(1.0, 1.0, reflectionParams.w)).y + (v46 * vec3(1.0, 1.0, reflectionParams.w)).z * (v46 * vec3(1.0, 1.0, reflectionParams.w)).z));
  vec3 v77 = v43 + v76 * 128.0;
  vec4 v78 = (v2.uPMatrix * v2.uLookAtMat) * vec4(v77, 1.0);
  v78 /= v78.w;
  vec2 v79 = v78.xy * 0.5 + 0.5;
  vec3 v80 = texture(liquidBackBuffer, v79).rgb;
  float v81 = (1.0 / 0.75) * min(max(0.75 - v76.z * v76.z, 0.0), 1.0);
  float v82 = min(max(1.0 - abs(v78.x * v78.x * v78.x * v78.x), 0.0), 1.0) * min(max(1.0 - v78.y * v78.y * v78.y * v78.y, 0.0), 1.0) * v81;
  vec3 v83 = v3.color_and_heightRate.rgb;
  v70 = v83 + (v80 - v83) * min(max(v82, 0.0), 1.0);
  float v85 = v70.rgb.x * vec3(0.3, 0.6, 0.1).x + v70.rgb.y * vec3(0.3, 0.6, 0.1).y + v70.rgb.z * vec3(0.3, 0.6, 0.1).z;
  v63 = v63 * v63;
  vec3 v86 = 0.5 * min(max(v63 + 0.9, 0.0), 1.0) * (v70 + (vec3(v85) - v70) * 0.3);
  v86 = v86 + 0.5 * v63 * (v70 + (vec3(v85) - v70) * 0.7);
  vec3 v87 = (v2.uInvLookAtMat * vec4(-v2.extLight.uExteriorDirectColorDir.xyz, 0.0)).xyz / sqrt((v2.uInvLookAtMat * vec4(-v2.extLight.uExteriorDirectColorDir.xyz, 0.0)).xyz.x * (v2.uInvLookAtMat * vec4(-v2.extLight.uExteriorDirectColorDir.xyz, 0.0)).xyz.x + (v2.uInvLookAtMat * vec4(-v2.extLight.uExteriorDirectColorDir.xyz, 0.0)).xyz.y * (v2.uInvLookAtMat * vec4(-v2.extLight.uExteriorDirectColorDir.xyz, 0.0)).xyz.y + (v2.uInvLookAtMat * vec4(-v2.extLight.uExteriorDirectColorDir.xyz, 0.0)).xyz.z * (v2.uInvLookAtMat * vec4(-v2.extLight.uExteriorDirectColorDir.xyz, 0.0)).xyz.z);
  vec3 v89 = v2.extLight.uSunPosition.xyz;
  vec3 v90 = v2.extLight.uSunAttenuation.xyz;
  float v92 = 1.0;
  if (v90.z > 0.0)
  {
    vec3 v93 = v89 - v5;
    float v94 = v93.x * v93.x + v93.y * v93.y + v93.z * v93.z;
    float v95 = v94 * (1.0f / sqrt(v94));
    v92 = 1.0 - min(max((v95 - v90.x) * v90.y, 0.0), 1.0);
  }
  float v97;
  float v98 = v2.extLight.uExteriorSpecularColor.a;
  vec3 v100 = -v45;
  vec3 v102 = (v87 + v100) / sqrt((v87 + v100).x * (v87 + v100).x + (v87 + v100).y * (v87 + v100).y + (v87 + v100).z * (v87 + v100).z);
  float v103 = min(max(v102.x * v46.x + v102.y * v46.y + v102.z * v46.z, 0.0), 1.0);
  v97 = pow(v103, v98);
  float v104 = v97 * v92;
  v86 = v86 + 0.2 + 0.3 * (v104 + v104 * v70) * v63 + 0.3 * v85 * v63;
  v68 = v68 * v41;
  vec4 v105 = makeFog2(v3, vec4(v86, v68), v2.uViewUpSceneTime.xyz, v5, v2.extLight.uExteriorDirectColorDir.xyz, 2);
  return vec4(v105.rgb, v68);
}

#endif //LIQUID_MERCURY_MATERIAL_SLANG
