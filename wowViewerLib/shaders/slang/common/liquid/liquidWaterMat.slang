#ifndef LIQUID_WATER_MATERIAL_SLANG
#define LIQUID_WATER_MATERIAL_SLANG

import glsl;

#include "../commonLiquidStructs.slang"
#include "./commonLiquidFunctions.slang"
#include "../commonLightFunctions.slang"
#include "../commonFogFunctions.slang"
#include "../commonGBufferData.slang"

vec4 calcWaterLiquidMat(in LiquidInstanceData v0, in WaterData v1, in SceneWideParams v2, in PSFog v3, in vec3 v4, in vec3 v5, in vec2 v6, in vec2 v7, in vec2 v8, in float v9, in sampler2D v10, in sampler2D v11)
{
  float v12 = v2.uViewUpSceneTime.w;
  bool v13 = v0.liquidMaterial_liquidType_isInterior_vtxCellOffetWidthHeight.z > 0;
  float v14 = v0.flowSpeed_flowDirection_unused4_unused5.x;
  float v15 = v0.flowSpeed_flowDirection_unused4_unused5.y;
  vec2 v16;
  float v17 = v15 * 0.017453292f;
  float v18 = v14 + 0.015f;
  v16.x = cos(v17) * v18;
  v16.y = sin(v17) * v18;
  vec2 v19 = GetTexScrollMtx(v12, v16);
  vec4 v20;
  vec4 v21;
  vec4 v22;
  vec4 v23;
  {
    vec2 v28 = v4.xy;
    mat4 v24[4];
    vec4 v25;
    vec4 v26;
    vec4 v27;
    float v30 = v1.f0_f1_f2_f3.z;
    float v31 = v1.f0_f1_f2_f3.w;
    float v32 = v1.f4_f5_f6_f7.x;
    float v33 = v1.f4_f5_f6_f7.y;
    float v34 = v1.f8_f9_f10_f11.x;
    float v35 = v1.f16_f17_pad_pad.x;
    float v36 = v1.computedOffset0_computedOffsets1_pad_pad.x;
    float v37 = v1.computedOffset0_computedOffsets1_pad_pad.y;
    float v38 = v33 * 0.017453292f;
    float v39 = cos(v38);
    float v40 = sin(v38);
    v24[0] = mat4(vec4(v39 * v31, v40 * v31, 0.0, 0.0), vec4(-v40 * v31, v39 * v31, 0.0, 0.0), vec4(0.0, 0.0, v31, 0.0), vec4(0.0, 0.0, 0.0, 1.0));
    v24[1] = mat4(vec4(v39 * v32, v40 * v32, 0.0, 0.0), vec4(-v40 * v32, v39 * v32, 0.0, 0.0), vec4(0.0, 0.0, v32, 0.0), vec4(0.0, 0.0, 0.0, 1.0));
    v24[2] = mat4(vec4(1.0, 0.0, 0.0, 0.0), vec4(0.0, v30, 0.0, 0.0), vec4(0.0, 0.0, 1.0, 0.0), vec4(0.0, 0.0, 0.0, 1.0));
    v24[3] = mat4(vec4(1.0, 0.0, 0.0, 0.0), vec4(0.0, 1.0, 0.0, 0.0), vec4(0.0, 0.0, 1.0, 0.0), vec4(v19.x, v19.y, 0.0, 1.0));
    float v41 = v35 * 0.0018750001f;
    float v42 = 0.0;
    float v43 = 0.0;
    if (v41 != 0.0f)
    {
      float v48 = 100.0f * v12;
      float v49 = v48 * v41;
      float v50 = v36 * 100000.0f;
      float v51 = v49 - (round(v49 / v50) * v50);
      float v52 = v51 * 0.000010000001f;
      v42 = v52;
      float v57 = 100.0f * v12;
      float v58 = v57 * v41;
      float v59 = v37 * 100000.0f;
      float v60 = v58 - (round(v58 / v59) * v59);
      float v61 = v60 * 0.000010000001f;
      v43 = v61;
    }
    v25 = vec4(mod(v12, 10000.0f) * 0.00031415926f, v31 * v42, v32 * v43, v34);
    v26 = vec4(-0.001875f, -0.001875f, 0.0, 0.0);
    v27 = vec4(v28.x, v28.y, 0.0, 0.0);
    v20.xy = (v24[0] * vec4(v7.xy / 32.0, 0.0, 1.0)).xy;
    v20.zw = (v24[1] * vec4(-(v7.xy + 0.5) / 32.0, 0.0, 1.0)).xy;
    vec4 v63 = vec4(4.0, 2.0, -1.0, -1.0);
    v63 = v63 * v25.x + v4.xyxy;
    const float v64 = 57.0 * 4.0;
    v22 = v20.xyxy * v64;
    float v65 = sin(v63.x);
    float v66 = cos(v63.y);
    vec2 v67 = vec2(0.1 * v65, 0.1 * v66);
    vec2 v68 = vec2(0.05 * v65 + 0.5, 0.05 * v66 + 0.5);
    v22 = v22 + vec4(v67, v68);
    v23.xy = (v24[2] * vec4(v8, 0.0, 1.0)).xy;
    v23.zw = v8;
    v20.xy = v20.xy + vec2(v25.y, 0.0);
    v20.zw = v20.zw + vec2(-v25.z, 0.0);
    v21.xy = (v24[3] * vec4(32.0 * v25.w * v20.xy, 0.0, 1.0)).yx;
    v21.zw = (v24[3] * vec4(32.0 * v25.w * v20.zw, 0.0, 1.0)).yx;
    v20.xy = (v24[3] * vec4(v20.xy, 0.0, 1.0)).yx;
    v20.zw = (v24[3] * vec4(v20.zw, 0.0, 1.0)).yx;
  }
  vec4 v29;
  vec4 v44;
  vec4 v45;
  vec4 v46;
  vec4 v47;
  vec4 v53;
  vec4 v54;
  vec4 v55;
  float v73 = v1.f0_f1_f2_f3.z;
  float v74 = v1.f4_f5_f6_f7.z;
  float v75 = v1.f4_f5_f6_f7.w;
  float v76 = v1.f8_f9_f10_f11.y;
  float v77 = v1.f8_f9_f10_f11.z;
  float v78 = v1.f8_f9_f10_f11.w;
  float v79 = v1.f12_f13_f14_f15.x;
  float v80 = v1.f12_f13_f14_f15.y;
  float v81 = v1.f12_f13_f14_f15.z;
  float v82 = v1.f12_f13_f14_f15.w;
  v29 = vec4(0.0);
  v44 = vec4(0.0);
  v45 = vec4(0.0, v75 * 0.35f, 1.0, v73);
  v46 = vec4(0.0);
  v47 = vec4(v80, v81, v82, v74);
  v53 = vec4(v79, v78, v76, !v13 ? v77 : 0.0f);
  v54 = vec4(0.0, 0.0, 0.0, 0.0);
  v55 = v1.depthTable;
  v46 = v2.extLight.uSunPosition;
  vec3 v84 = (v2.uInvLookAtMat * vec4(0.0, 0.0, 0.0, 1.0)).xyz;
  vec3 v85 = v4 - v84;
  float v86 = sqrt(v85.x * v85.x + v85.y * v85.y + v85.z * v85.z);
  vec3 v87 = v85 / v86;
  vec3 v88 = vec3(0.0, 0.0, 1.0);
  vec2 v89 = v6 / screenSize.xy;
  float v90 = texture(depthTex, v6 / screenSize.xy).r;
  float v91 = abs(sceneViewDepth(v6, v2));
  float v92 = abs(v5.z);
  float v93 = (v90 <= 0.0001) ? 100000.0 : (v91 - v92);
  float v94 = v93 / max(v92, 0.0001);
  vec3 v95;
  float v100 = v47.w;
  vec2 v101 = texture(v10, v20.xy).xy + texture(v10, v20.zw).xy - 1.0;
  v101 = v101 * 2.0;
  vec2 v102 = texture(v10, v21.xy).xy + texture(v10, v21.zw).xy - 1.0;
  v102 = v102 * 2.0;
  vec3 v103 = vec3(0.7 * v102 + (v101 - 0.7 * v102) * min(max(0.2 + v86 / 1000.0, 0.0), 1.0), 1.0);
  v103.xy = v103.xy * v100;
  v95 = v103 / sqrt(v103.x * v103.x + v103.y * v103.y + v103.z * v103.z);
  vec3 v105 = v88;
  v105.xy = v105.xy + v47.y * v95.xy;
  v105 = v105 / sqrt(v105.x * v105.x + v105.y * v105.y + v105.z * v105.z);
  float v106;
  vec3 v107 = -v87;
  float v109 = 1.0 - min(max(v107.x * v105.x + v107.y * v105.y + v107.z * v105.z, 0.0), 1.0);
  v106 = v109 * v109;
  vec3 v111 = (v2.uInvLookAtMat * vec4(-v2.extLight.uExteriorDirectColorDir.xyz, 0.0)).xyz / sqrt((v2.uInvLookAtMat * vec4(-v2.extLight.uExteriorDirectColorDir.xyz, 0.0)).xyz.x * (v2.uInvLookAtMat * vec4(-v2.extLight.uExteriorDirectColorDir.xyz, 0.0)).xyz.x + (v2.uInvLookAtMat * vec4(-v2.extLight.uExteriorDirectColorDir.xyz, 0.0)).xyz.y * (v2.uInvLookAtMat * vec4(-v2.extLight.uExteriorDirectColorDir.xyz, 0.0)).xyz.y + (v2.uInvLookAtMat * vec4(-v2.extLight.uExteriorDirectColorDir.xyz, 0.0)).xyz.z * (v2.uInvLookAtMat * vec4(-v2.extLight.uExteriorDirectColorDir.xyz, 0.0)).xyz.z);
  vec3 v113 = v2.extLight.uSunPosition.xyz;
  vec3 v114 = v2.extLight.uSunAttenuation.xyz;
  float v116 = 1.0;
  if (v114.z > 0.0)
  {
    vec3 v117 = v113 - v5;
    float v118 = v117.x * v117.x + v117.y * v117.y + v117.z * v117.z;
    float v119 = v118 * (1.0f / sqrt(v118));
    v116 = 1.0 - min(max((v119 - v114.x) * v114.y, 0.0), 1.0);
  }
  float v121;
  v121 = min(max(v111.x * v88.x + v111.y * v88.y + v111.z * v88.z, 0.0), 1.0);
  float v124 = v121 * v116;
  vec3 v125 = (v2.extLight.uExteriorDirectColor.rgb * v124) + v2.extLight.uExteriorAmbientColor.rgb;
  vec3 v126 = v125 * vec3(1.0);
  int v127 = v1.waterType_pad_pad_pad.x;
  vec4 v128;
  vec4 v129;
  if (v127 == 0)
  {
    v128 = v2.closeOceanColor_shallowAlpha;
    v129 = v2.farOceanColor_deepAlpha;
  }
  else
  {
    v128 = v2.closeRiverColor_shallowAlpha;
    v129 = v2.farRiverColor_deepAlpha;
    if (v127 == 2 && v13)
    {
      v128.rgb = vec3(1.0);
      v129.rgb = vec3(1.0);
    }
  }
  float v131 = (v9 == 0.0) ? 0.0 : 1.0;
  float v132;
  float v135 = v9 * v9;
  float v136 = v55.x + v55.y * v9;
  float v137 = v135 * v9;
  v136 = v136 + v135 * v55.z + v137 * v55.w;
  v132 = min(max(v136, 0.0), 1.0);
  float v138 = v132 * v45.w;
  v138 = v138 + abs(0.1 * v95.y);
  vec4 v139 = v128 + (v129 - v128) * v138;
  vec4 v140 = vec4(v139.rgb * v126, v139.a);
  vec2 v141 = v89 + 0.1 * v105.xy * min(max(100.0 * v94 * (1.2 - v106), 0.0), 1.0);
  float v142 = abs(sceneViewDepth(v141 * screenSize.xy, v2));
  bool v143 = v142 > v92;
  v141 = v143 ? v141 : v89;
  vec3 v144 = texture(liquidBackBuffer, v141).rgb;
  float v145 = v143 ? (v142 - v92) : v93;
  vec4 v148 = underWaterFog;
  vec4 v149 = underWaterClassicFogParams;
  float v150;
  if (v149.x != 0.0)
  {
    float v151 = max(0.0, (v149.y - v145) * v149.z);
    v150 = min(max(pow(v151, v149.w), 0.0), 1.0);
  }
  else
  {
    float v152 = max(0.0, (v145 - v148.w) - v148.x);
    float v153 = 1.0 / exp(v152 * v148.z);
    float v154 = v145 / v148.y;
    float v155 = min(max((1.0 / 0.7) * (1.0 - v154), 0.0), 1.0);
    v150 = min(v153, v155);
  }
  vec3 v157 = underWaterFogColor.rgb + (v144 - underWaterFogColor.rgb) * v150;
  v157 = v157 + (v140.rgb - v157) * ((1.0 - v106 * 0.1) * v139.a);
  float v159;
  float v160 = v2.extLight.uExteriorSpecularColor.a;
  vec3 v162 = -v87;
  vec3 v164 = (v111 + v162) / sqrt((v111 + v162).x * (v111 + v162).x + (v111 + v162).y * (v111 + v162).y + (v111 + v162).z * (v111 + v162).z);
  float v165 = min(max(v164.x * v105.x + v164.y * v105.y + v164.z * v105.z, 0.0), 1.0);
  v159 = pow(v165, v160);
  float v166 = v159 * v116 + min(max(v105.z - 0.8, 0.0), 1.0);
  vec3 v167 = v2.extLight.uExteriorSpecularColor.rgb * v166;
  vec3 v168 = v167 * v106;
  float v169 = min(max((v93 - 0.0) / (1.2 * min(max(v106 * 0.5 + 0.2, 0.0), 1.0) - 0.0), 0.0f), 1.0f) * min(max((v93 - 0.0) / (1.2 * min(max(v106 * 0.5 + 0.2, 0.0), 1.0) - 0.0), 0.0f), 1.0f) * (3.0f - 2.0f * min(max((v93 - 0.0) / (1.2 * min(max(v106 * 0.5 + 0.2, 0.0), 1.0) - 0.0), 0.0f), 1.0f));
  float v170 = 1.0 - min(max((v93 * (1.0 - v87.z) - 0.0) / (2.5 * min(max(v106 * 0.5 + 0.2, 0.0), 1.0) - 0.0), 0.0f), 1.0f) * min(max((v93 * (1.0 - v87.z) - 0.0) / (2.5 * min(max(v106 * 0.5 + 0.2, 0.0), 1.0) - 0.0), 0.0f), 1.0f) * (3.0f - 2.0f * min(max((v93 * (1.0 - v87.z) - 0.0) / (2.5 * min(max(v106 * 0.5 + 0.2, 0.0), 1.0) - 0.0), 0.0f), 1.0f));
  float v171 = v45.y * v170;
  vec4 v172 = texture(v11, v22.xy - v105.xy);
  v172 = v172 + texture(v11, v22.zw + 0.2 * v105.yx);
  float v173 = (v171 * (1.1 - min(max(v86 / 150.0, 0.0), 1.0))) * v105.z;
  v172.rgb = v173 * v172.rgb;
  v157 = v157 + v172.rgb;
  v169 = (v169 + (1.0 - v169) * min(max(-v93, 0.0), 1.0)) * v131;
  float v174;
  v174 = min(max(v111.x * v105.x + v111.y * v105.y + v111.z * v105.z, 0.0), 1.0);
  v157 = v157 + v53.z * v174 * v116 * v2.extLight.uExteriorDirectColor.rgb * v140.rgb + v53.w * v168;
  vec4 v177 = makeFog2(v3, vec4(v157, v169), v2.uViewUpSceneTime.xyz, v5, v2.extLight.uExteriorDirectColorDir.xyz, 2);
  return vec4(v177.rgb, v169);
}

#endif //LIQUID_WATER_MATERIAL
