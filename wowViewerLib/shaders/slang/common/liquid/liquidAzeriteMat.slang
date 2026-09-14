#ifndef LIQUID_AZERITHE_MATERIAL_SLANG
#define LIQUID_AZERITHE_MATERIAL_SLANG

import glsl;

#include "../commonLiquidStructs.slang"
#include "./commonLiquidFunctions.slang"
#include "../commonFogFunctions.slang"

vec4 calcAzeritheLiquidMat(in LiquidInstanceData v0, in AzeritheData v1, in SceneWideParams v2, in PSFog v3, in vec3 v4, in vec3 v5, in vec2 v6, in vec2 v7, in float v8, in sampler2D v9, in sampler2D v10, in sampler2D v11, in sampler2D v12, in sampler2D v13, in sampler2D v14, in sampler2D v15, in sampler2D v16)
{
  float v17 = v2.uViewUpSceneTime.w;
  float v18 = v0.flowSpeed_flowDirection_unused4_unused5.x;
  float v19 = v0.flowSpeed_flowDirection_unused4_unused5.y;
  mat4 v20[4];
  mat4 v21[4];
  float v26 = v1.f0_f1_f2_f3.x;
  float v27 = v1.f0_f1_f2_f3.y;
  float v28 = v1.f0_f1_f2_f3.z;
  float v29 = v1.f0_f1_f2_f3.w;
  float v30 = v1.f4_f5_f6_f7.x;
  float v31 = v1.f4_f5_f6_f7.y;
  mat4 v32;
  float v38 = v17;
  float v39 = (v19 + v30) * 0.017453292f;
  float v40 = v31 + v18;
  vec2 v41 = vec2(cos(v39) * v40, sin(v39) * v40);
  vec2 v42 = GetTexScrollMtx(v38, v41);
  v32 = mat4(vec4(v27, 0.0, 0.0, 0.0), vec4(0.0, v27, 0.0, 0.0), vec4(0.0, 0.0, v27, 0.0), vec4(v42.x, v42.y, 0.0, 1.0));
  v21[0] = v32;
  mat4 v43;
  float v49 = v17;
  float v50 = (v19 + v30) * 0.017453292f;
  float v51 = v31 + v18;
  vec2 v52 = vec2(cos(v50) * v51, sin(v50) * v51);
  vec2 v53 = GetTexScrollMtx(v49, v52);
  v43 = mat4(vec4(v29, 0.0, 0.0, 0.0), vec4(0.0, v29, 0.0, 0.0), vec4(0.0, 0.0, v29, 0.0), vec4(v53.x, v53.y, 0.0, 1.0));
  v21[1] = v43;
  mat4 v54;
  float v60 = v17;
  float v61 = (v19 + v30) * 0.017453292f;
  float v62 = v31 + v18;
  vec2 v63 = vec2(cos(v61) * v62, sin(v61) * v62);
  vec2 v64 = GetTexScrollMtx(v60, v63);
  v54 = mat4(vec4(v26, 0.0, 0.0, 0.0), vec4(0.0, v26, 0.0, 0.0), vec4(0.0, 0.0, v26, 0.0), vec4(v64.x, v64.y, 0.0, 1.0));
  v21[2] = v54;
  mat4 v65;
  float v71 = v17;
  float v72 = (v19 + v30) * 0.017453292f;
  float v73 = v31 + v18;
  vec2 v74 = vec2(cos(v72) * v73, sin(v72) * v73);
  vec2 v75 = GetTexScrollMtx(v71, v74);
  v65 = mat4(vec4(v28, 0.0, 0.0, 0.0), vec4(0.0, v28, 0.0, 0.0), vec4(0.0, 0.0, v28, 0.0), vec4(v75.x, v75.y, 0.0, 1.0));
  v21[3] = v65;
  v20[0] = v21[0];
  v20[1] = v21[1];
  v20[2] = v21[2];
  v20[3] = v21[3];
  mat4 v22[4];
  v22[0] = v20[0];
  v22[1] = v20[1];
  v22[2] = v20[2];
  v22[3] = v20[3];
  float v77 = v1.f8_f9_f10_f11.x;
  float v78 = v1.f8_f9_f10_f11.y;
  float v79 = v1.f8_f9_f10_f11.w;
  float v80 = v1.f12_f13_f14_f15.x;
  float v81 = v1.f12_f13_f14_f15.z;
  float v82 = v1.f12_f13_f14_f15.w;
  vec4 v84 = vec4(v77, (v79 > 0.0) ? 1.0 / v79 : 1.0, v81, v82);
  float v85 = 0.0;
  if (v78 > 0.0)
  {
    v85 = mod(v17, v78 * 1000.0) / (v78 * 1000.0);
  }
  float v86 = v80 * 1000.0;
  float v87 = v1.color0.w;
  float v88 = 0.0;
  if (v86 > 0.0 && v87 > 1.0)
  {
    v88 = fract(v87 * mod(v17, v86) / v86);
  }
  vec2 v89 = (v22[0] * vec4(v7, 0.0, 1.0)).xy;
  vec2 v90 = (v22[1] * vec4(v7, 0.0, 1.0)).xy;
  vec2 v91 = (v22[2] * vec4(v7, 0.0, 1.0)).xy;
  vec2 v92 = (v22[3] * vec4(v7, 0.0, 1.0)).xy;
  float v94 = (v8 == 0.0) ? 0.0 : 1.0;
  float v95 = min(max(v8, 0.0), 1.0);
  vec2 v97 = vec2(0.0, 0.0);
  vec2 v98 = texture(v13, v91 + v97).xy * 2.0 - 1.0;
  vec2 v99 = texture(v14, v91 + v97).xy * 2.0 - 1.0;
  vec3 v100 = vec3(v98.x, v98.y, sqrt(1.0 - (v98.x * v98.x + v98.y * v98.y)));
  vec3 v101 = vec3(v99.x, v99.y, sqrt(1.0 - (v99.x * v99.x + v99.y * v99.y)));
  vec3 v102 = v100 + (v101 - v100) * v88;
  v102 = vec3(0.0, 0.0, 1.0) + (v102 - vec3(0.0, 0.0, 1.0)) * v84.w;
  if (sqrt(v102.x * v102.x + v102.y * v102.y + v102.z * v102.z) < 0.006)
  {
    v102.z = v102.z + 0.001;
  }
  v102 = v102 / sqrt(v102.x * v102.x + v102.y * v102.y + v102.z * v102.z);
  vec2 v103 = v84.x * v102.xy;
  vec3 v104 = texture(v11, v91 + v103).rgb;
  float v105 = v104.y;
  float v106 = v104.z;
  vec4 v107 = texture(v9, v91 + v103);
  vec4 v108 = texture(v12, v89 + v103);
  vec4 v109 = texture(v10, v92 + v103);
  float v111 = v104.r;
  if (v106 < v105)
  {
    if (v85 < v105)
    {
      v85 = v85 + 1.0;
    }
    v106 = v106 + 1.0;
  }
  float v115 = min(max(v85 - v105, 0.0), 1.0) / (v106 - v105);
  float v116 = 0.0;
  if (!(v115 > 1.0 || v111 > 1.0))
  {
    float v117 = abs(2.0 * v115 - 1.0);
    v116 = min(max(v111 - v117, 0.0), 1.0);
  }
  float v119 = min(max(1.0 - v116 / 0.5, 0.0), 1.0);
  float v120 = min(max(v116 / 0.5, 0.0), 1.0) - min(max((v116 - 0.5) / 0.25, 0.0), 1.0);
  float v121 = min(max((v116 - 0.5) / 0.25, 0.0), 1.0) - min(max((v116 - 0.75) / 0.25, 0.0), 1.0);
  float v122 = min(max((v116 - 0.75) / 0.25, 0.0), 1.0);
  float v123 = 1.0 - min(max((1.0 - v95) * v84.z, 0.0), 1.0);
  vec3 v124 = v109.rgb * v121;
  vec3 v125 = (v2.uInvLookAtMat * vec4(0.0, 0.0, 0.0, 1.0)).xyz;
  vec3 v126 = v4 - v125;
  vec3 v127 = (v2.uInvLookAtMat * vec4(-v2.extLight.uExteriorDirectColorDir.xyz, 0.0)).xyz / sqrt((v2.uInvLookAtMat * vec4(-v2.extLight.uExteriorDirectColorDir.xyz, 0.0)).xyz.x * (v2.uInvLookAtMat * vec4(-v2.extLight.uExteriorDirectColorDir.xyz, 0.0)).xyz.x + (v2.uInvLookAtMat * vec4(-v2.extLight.uExteriorDirectColorDir.xyz, 0.0)).xyz.y * (v2.uInvLookAtMat * vec4(-v2.extLight.uExteriorDirectColorDir.xyz, 0.0)).xyz.y + (v2.uInvLookAtMat * vec4(-v2.extLight.uExteriorDirectColorDir.xyz, 0.0)).xyz.z * (v2.uInvLookAtMat * vec4(-v2.extLight.uExteriorDirectColorDir.xyz, 0.0)).xyz.z);
  vec3 v129 = v2.extLight.uSunPosition.xyz;
  vec3 v130 = v2.extLight.uSunAttenuation.xyz;
  float v132 = 1.0;
  if (v130.z > 0.0)
  {
    vec3 v133 = v129 - v5;
    float v134 = v133.x * v133.x + v133.y * v133.y + v133.z * v133.z;
    float v135 = v134 * (1.0f / sqrt(v134));
    v132 = 1.0 - min(max((v135 - v130.x) * v130.y, 0.0), 1.0);
  }
  float v137;
  v137 = min(max(v127.x * v102.x + v127.y * v102.y + v127.z * v102.z, 0.0), 1.0);
  float v140 = v137 * v132;
  vec3 v141 = (v2.extLight.uExteriorDirectColor.rgb * v140) + v2.extLight.uExteriorAmbientColor.rgb;
  vec3 v142 = v141 * 0.25 + vec3(0.75, 0.75, 0.75);
  v124 = v124 + v142 * v108.rgb * v120 + v142 * v107.rgb * v119;
  vec2 v143;
  vec3 v146 = v126 - 2.0 * (v102.x * v126.x + v102.y * v126.y + v102.z * v126.z) * v102;
  float v147 = 2.0 * sqrt(v146.x * v146.x + v146.y * v146.y + (v146.z + 1.0) * (v146.z + 1.0));
  v143 = v146.xy / v147 + 0.5;
  v124 = v124 + min(max(texture(v15, v143).rgb, 0.0), 1.0) * v119 * v107.a;
  v124 = v124 + 2.0 * v109.rgb * v122;
  vec3 v148 = v1.color0.rgb;
  v124 = v124 + (v142 * v148.rgb - v124) * v123;
  float v149 = abs(sceneViewDepth(v6, v2));
  float v150 = v84.y;
  float v151 = min(max(v150 * (v149 - abs(v5.z)), 0.0), 1.0);
  vec2 v152 = vec2(fract(v90.x * 0.1), 1.0 - v151);
  vec4 v153 = texture(v16, v152);
  v124 = v153.rgb + (v124 - v153.rgb) * v151;
  float v155 = v94 * sqrt(v151);
  vec4 v156 = makeFog2(v3, vec4(v124, v94), v2.uViewUpSceneTime.xyz, v5, v2.extLight.uExteriorDirectColorDir.xyz, 2);
  return vec4(v156.rgb, v155);
}

#endif //LIQUID_AZERITHE_MATERIAL_SLANG
