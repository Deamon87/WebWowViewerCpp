#ifndef LIQUID_FEL_MATERIAL_SLANG
#define LIQUID_FEL_MATERIAL_SLANG

import glsl;

#include "../commonLiquidStructs.slang"
#include "./commonLiquidFunctions.slang"
#include "../commonFogFunctions.slang"

vec4 calcFelLiquidMat(in LiquidInstanceData v0, in FelData v1, in SceneWideParams v2, in PSFog v3, in vec3 v4, in vec3 v5, in vec2 v6, in vec2 v7, in float v8, in sampler2D v9, in sampler2D v10, in sampler2D v11, in sampler2D v12, in sampler2D v13, in sampler2D v14, in sampler2D v15)
{
  float v16 = v2.uViewUpSceneTime.w;
  float v17 = v0.flowSpeed_flowDirection_unused4_unused5.x;
  float v18 = v0.flowSpeed_flowDirection_unused4_unused5.y;
  float v21 = v16;
  mat4 v19[5];
  float v25 = v1.f0_f1_f2_f3.x;
  float v26 = v1.f0_f1_f2_f3.y;
  float v27 = v1.f0_f1_f2_f3.z;
  float v28 = v1.f0_f1_f2_f3.w;
  float v29 = v1.f4_f5_f6_f7.x;
  float v30 = v1.f4_f5_f6_f7.y;
  float v31 = v1.f4_f5_f6_f7.z;
  float v32 = v1.f4_f5_f6_f7.w;
  float v33 = v1.f8_f9_f10_f11.x;
  float v34 = v1.f8_f9_f10_f11.y;
  float v35 = v1.f8_f9_f10_f11.z;
  float v36 = v1.intParams.y;
  float v37 = v1.intParams.z;
  mat4 v38;
  float v44 = v21;
  float v45 = (v18 + v26) * 0.017453292f;
  float v46 = v27 * v17;
  vec2 v47 = vec2(cos(v45) * v46, sin(v45) * v46);
  vec2 v48 = GetTexScrollMtx(v44, v47);
  v38 = mat4(vec4(v25, 0.0, 0.0, 0.0), vec4(0.0, v25, 0.0, 0.0), vec4(0.0, 0.0, v25, 0.0), vec4(v48.x, v48.y, 0.0, 1.0));
  v19[0] = v38;
  mat4 v49;
  float v55 = v21;
  float v56 = (v18 + v29) * 0.017453292f;
  float v57 = v30 * v17;
  vec2 v58 = vec2(cos(v56) * v57, sin(v56) * v57);
  vec2 v59 = GetTexScrollMtx(v55, v58);
  v49 = mat4(vec4(v28, 0.0, 0.0, 0.0), vec4(0.0, v28, 0.0, 0.0), vec4(0.0, 0.0, v28, 0.0), vec4(v59.x, v59.y, 0.0, 1.0));
  v19[1] = v49;
  mat4 v60;
  float v66 = v21;
  float v67 = (v18 + v32) * 0.017453292f;
  float v68 = v33 * v17;
  vec2 v69 = vec2(cos(v67) * v68, sin(v67) * v68);
  vec2 v70 = GetTexScrollMtx(v66, v69);
  v60 = mat4(vec4(v31, 0.0, 0.0, 0.0), vec4(0.0, v31, 0.0, 0.0), vec4(0.0, 0.0, v31, 0.0), vec4(v70.x, v70.y, 0.0, 1.0));
  v19[2] = v60;
  float v71 = v18 * 0.017453292f;
  float v72 = v35 * v17;
  vec2 v73 = vec2(cos(v71) * v72, sin(v71) * v72);
  float v74 = v34 * (v37 + 1.0);
  vec2 v75 = GetTexScrollMtx(v21, v73);
  v19[3] = mat4(vec4(v74, 0.0, 0.0, 0.0), vec4(0.0, v74, 0.0, 0.0), vec4(0.0, 0.0, v74, 0.0), vec4(v75.x, v75.y, 0.0, 1.0));
  float v76 = v71 + 3.1415927f;
  vec2 v77 = vec2(cos(v76) * v72, sin(v76) * v72);
  float v78 = v34 * (1.0 - v37);
  vec2 v79 = GetTexScrollMtx(v21, v77);
  float v80 = sin(v76) * v36;
  float v81 = cos(v76) * v36;
  v19[4] = mat4(vec4(v78, 0.0, 0.0, 0.0), vec4(0.0, v78, 0.0, 0.0), vec4(0.0, 0.0, v78, 0.0), vec4(v79.x + v80, v79.y + v81, 0.0, 1.0));
  float v85 = 1.0f;
  vec4 v20;
  vec4 v22;
  vec4 v23;
  float v87 = v1.f8_f9_f10_f11.w;
  float v88 = v1.f12_f13_f14_f15.x;
  float v89 = v1.f12_f13_f14_f15.y;
  float v90 = v1.f12_f13_f14_f15.z;
  float v91 = v1.f12_f13_f14_f15.w;
  float v92 = v1.f16_f17_pad_pad.x;
  float v93 = v1.f16_f17_pad_pad.y;
  float v94 = v1.intParams.x;
  float v95 = 1.0;
  if (v87 > 0.0)
  {
    v95 = 1.0 / v87;
  }
  v20 = vec4(v89, v95, v90, v91);
  v22 = vec4(v88 * 1000.0, v92, v93, 0.0);
  v23 = vec4(v94, 0.0, v85, 0.0);
  float v97 = v1.f12_f13_f14_f15.x * 1000.0;
  float v98 = v1.intParams.w;
  float v99 = 0.0;
  if (v97 > 0.0 && v98 > 1.0)
  {
    v99 = fract(v98 * mod(v16, v97) / v97);
  }
  v22.x = v99;
  vec2 v100 = (v19[0] * vec4(v7, 0.0, 1.0)).xy;
  vec2 v101 = (v19[1] * vec4(v7, 0.0, 1.0)).xy;
  vec2 v102 = (v19[2] * vec4(v7, 0.0, 1.0)).xy;
  vec2 v103 = (v19[3] * vec4(v7, 0.0, 1.0)).xy;
  vec2 v104 = (v19[4] * vec4(v7, 0.0, 1.0)).xy;
  float v106 = (v8 == 0.0) ? 0.0 : 1.0;
  float v107;
  vec4 v108 = v1.depthTable;
  float v110 = v8 * v8;
  float v111 = v108.x + v108.y * v8;
  float v112 = v110 * v8;
  v111 = v111 + v110 * v108.z + v112 * v108.w;
  v107 = min(max(v111, 0.0), 1.0);
  vec2 v115 = texture(v14, v102).xy;
  vec2 v116 = texture(v15, v102).xy;
  vec3 v117 = vec3(v115.x, v115.y, sqrt(min(max(1.0 - (v115.x * v115.x + v115.y * v115.y), 0.0), 1.0)));
  vec3 v118 = vec3(v116.x, v116.y, sqrt(min(max(1.0 - (v116.x * v116.x + v116.y * v116.y), 0.0), 1.0)));
  vec3 v119 = v117 + (v118 - v117) * v22.x;
  v119 = 2.0 * v119 - 1.0;
  float v120 = v22.y;
  float v121 = 1.0 - v107 * v120;
  v119 = v119 + (vec3(0.0, 0.0, 1.0) - v119) * v121;
  vec2 v122 = v20.w * v119.xy;
  vec3 v123 = texture(v9, v100 + v122).rgb;
  vec3 v124 = texture(v10, v101 + v122).rgb;
  float v125 = v20.z;
  float v126 = 1.0 - v107 * v125;
  if (v23.x != 0.0)
  {
    v126 = min(max(v126, 0.0), 1.0);
  }
  vec3 v127 = v123 + (v124 - v123) * v126;
  vec3 v128 = (v2.uInvLookAtMat * vec4(0.0, 0.0, 0.0, 1.0)).xyz;
  vec3 v129 = (v2.uInvLookAtMat * vec4(-v2.extLight.uExteriorDirectColorDir.xyz, 0.0)).xyz / sqrt((v2.uInvLookAtMat * vec4(-v2.extLight.uExteriorDirectColorDir.xyz, 0.0)).xyz.x * (v2.uInvLookAtMat * vec4(-v2.extLight.uExteriorDirectColorDir.xyz, 0.0)).xyz.x + (v2.uInvLookAtMat * vec4(-v2.extLight.uExteriorDirectColorDir.xyz, 0.0)).xyz.y * (v2.uInvLookAtMat * vec4(-v2.extLight.uExteriorDirectColorDir.xyz, 0.0)).xyz.y + (v2.uInvLookAtMat * vec4(-v2.extLight.uExteriorDirectColorDir.xyz, 0.0)).xyz.z * (v2.uInvLookAtMat * vec4(-v2.extLight.uExteriorDirectColorDir.xyz, 0.0)).xyz.z);
  vec3 v131 = v2.extLight.uSunPosition.xyz;
  vec3 v132 = v2.extLight.uSunAttenuation.xyz;
  float v134 = 1.0;
  if (v132.z > 0.0)
  {
    vec3 v135 = v131 - v5;
    float v136 = v135.x * v135.x + v135.y * v135.y + v135.z * v135.z;
    float v137 = v136 * (1.0f / sqrt(v136));
    v134 = 1.0 - min(max((v137 - v132.x) * v132.y, 0.0), 1.0);
  }
  float v139;
  v139 = min(max(v129.x * v119.x + v129.y * v119.y + v129.z * v119.z, 0.0), 1.0);
  float v142 = v139 * v134;
  vec3 v143 = (v2.extLight.uExteriorDirectColor.rgb * v142) + v2.extLight.uExteriorAmbientColor.rgb;
  float v145 = v20.x;
  vec3 v146 = v143 * v127 + (v127 - v143 * v127) * v145;
  vec3 v147 = v4 - v128;
  float v148;
  float v149 = v2.extLight.uExteriorSpecularColor.a;
  vec3 v151 = -(v147 / sqrt(v147.x * v147.x + v147.y * v147.y + v147.z * v147.z));
  vec3 v153 = (v129 + v151) / sqrt((v129 + v151).x * (v129 + v151).x + (v129 + v151).y * (v129 + v151).y + (v129 + v151).z * (v129 + v151).z);
  float v154 = min(max(v153.x * v119.x + v153.y * v119.y + v153.z * v119.z, 0.0), 1.0);
  v148 = pow(v154, v149);
  float v155 = v148 * v134;
  vec4 v156 = texture(v11, v7 + v122);
  vec3 v157 = v2.extLight.uExteriorSpecularColor.rgb * v156.rgb * v155 * v156.a;
  v146 = v146 + v157;
  vec4 v158 = texture(v13, v103 + v122);
  vec4 v159 = texture(v13, v104 + v122);
  float v160 = v107 * v22.z;
  vec3 v161 = (v158.rgb + v159.rgb) * v158.a * v159.a * v160;
  v146 = v146 + v161;
  float v162 = 1.0;
  {
    float v163 = abs(sceneViewDepth(v6, v2));
    float v164 = v20.y;
    float v165 = min(max(v164 * (v163 - abs(v5.z)), 0.0), 1.0);
    vec2 v166 = vec2(v101.x, 1.0 - v165);
    vec4 v167 = texture(v12, v166);
    v162 = v167.a;
    v146 = v167.rgb + (v146 - v167.rgb) * (v165 * v167.a);
  }
  float v168 = v162 * v106;
  vec4 v169 = makeFog2(v3, vec4(v146, v168), v2.uViewUpSceneTime.xyz, v5, v2.extLight.uExteriorDirectColorDir.xyz, 2);
  return vec4(v169.rgb, 1.0f);
}

#endif //LIQUID_FEL_MATERIAL_SLANG
