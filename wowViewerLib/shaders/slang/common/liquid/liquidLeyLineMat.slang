#ifndef LIQUID_LEYLINE_MATERIAL_SLANG
#define LIQUID_LEYLINE_MATERIAL_SLANG

import glsl;

#include "../commonLiquidStructs.slang"
#include "./commonLiquidFunctions.slang"
#include "../commonFogFunctions.slang"

vec4 calcLeyLineLiquidMat(in LiquidInstanceData v0, in LeyLineData v1, in SceneWideParams v2, in PSFog v3, in vec3 v4, in vec3 v5, in vec2 v6, in vec2 v7, in vec2 v8, in float v9, in sampler2D v10, in sampler2D v11, in sampler2D v12, in sampler2D v13)
{
  float v14 = v2.uViewUpSceneTime.w;
  float v15 = v0.flowSpeed_flowDirection_unused4_unused5.x;
  float v16 = v0.flowSpeed_flowDirection_unused4_unused5.y;
  mat4 v17[2];
  float v23 = v1.f0_f1_f2_f3.w;
  float v24 = v1.f4_f5_f6_f7.x;
  float v25 = v1.f4_f5_f6_f7.y;
  float v26 = v1.f4_f5_f6_f7.z;
  float v27 = v1.f4_f5_f6_f7.w;
  float v28 = v1.f8_f9_f10_f11.x;
  mat4 v29;
  float v35 = v14;
  float v36 = (v16 + v24) * 0.017453292f;
  float v37 = v25 + v15;
  vec2 v38 = vec2(cos(v36) * v37, sin(v36) * v37);
  vec2 v39 = GetTexScrollMtx(v35, v38);
  v29 = mat4(vec4(v23, 0.0, 0.0, 0.0), vec4(0.0, v23, 0.0, 0.0), vec4(0.0, 0.0, v23, 0.0), vec4(v39.x, v39.y, 0.0, 1.0));
  v17[0] = v29;
  mat4 v40;
  float v46 = v14;
  float v47 = (v16 + v27) * 0.017453292f;
  float v48 = v28 + v15;
  vec2 v49 = vec2(cos(v47) * v48, sin(v47) * v48);
  vec2 v50 = GetTexScrollMtx(v46, v49);
  v40 = mat4(vec4(v26, 0.0, 0.0, 0.0), vec4(0.0, v26, 0.0, 0.0), vec4(0.0, 0.0, v26, 0.0), vec4(v50.x, v50.y, 0.0, 1.0));
  v17[1] = v40;
  vec4 v18;
  vec4 v19;
  vec4 v20;
  vec4 v21;
  vec4 v22;
  vec4 v30;
  vec4 v31;
  float v58 = v1.f8_f9_f10_f11.y;
  float v59 = v1.f8_f9_f10_f11.z;
  float v60 = v1.f8_f9_f10_f11.w;
  float v61 = v1.f12_f13_f14_f15.x;
  float v62 = v1.f12_f13_f14_f15.y;
  float v63 = v1.f12_f13_f14_f15.z;
  float v64 = v1.f12_f13_f14_f15.w;
  float v65 = v1.f16_f17_pad_pad.x;
  float v66 = v1.f16_f17_pad_pad.y;
  mat4 v67;
  float v73 = v14;
  float v74 = (v16 + v60) * 0.017453292f;
  float v75 = v61 + v15;
  vec2 v76 = vec2(cos(v74) * v75, sin(v74) * v75);
  vec2 v77 = GetTexScrollMtx(v73, v76);
  v67 = mat4(vec4(v59, 0.0, 0.0, 0.0), vec4(0.0, v59, 0.0, 0.0), vec4(0.0, 0.0, v59, 0.0), vec4(v77.x, v77.y, 0.0, 1.0));
  v18 = vec4(0.0, 0.0, v65, v64);
  v19 = vec4(v1.color0.rgb, v66);
  v20 = vec4(v63, 0.0, 1.0, v58);
  v21 = vec4(v1.color1.rgb, 0.0);
  v22 = vec4(mod(v14, 10000.0f) * 0.000099999997f, v62, 0.0, 0.0);
  v30 = vec4(v58, 0.0, 0.0, 0.0);
  v31 = vec4(0.0);
  vec4 v79;
  v79.xy = v7 / 32.0;
  v79.zw = -(v7 + 0.5) / 32.0;
  vec4 v80 = v79.xyxy * (57.0 * 4.0);
  vec4 v81;
  v81.xy = (v67 * vec4(32.0 * v22.y * v79.xy, 0.0, 1.0)).yx;
  v81.zw = (v67 * vec4(32.0 * v22.y * v79.zw, 0.0, 1.0)).yx;
  v79.xy = (v67 * vec4(v79.xy, 0.0, 1.0)).yx;
  v79.zw = (v67 * vec4(v79.zw, 0.0, 1.0)).yx;
  vec2 v82 = (v17[0] * vec4(v7, 0.0, 1.0)).xy;
  vec2 v83 = (v17[1] * vec4(v7, 0.0, 1.0)).xy;
  float v85 = (v9 == 0.0) ? 0.0 : 1.0;
  vec3 v86 = (v2.uInvLookAtMat * vec4(0.0, 0.0, 0.0, 1.0)).xyz;
  vec3 v87 = v4 - v86;
  float v88 = sqrt(v87.x * v87.x + v87.y * v87.y + v87.z * v87.z);
  vec3 v89 = v87 / v88;
  vec3 v90 = vec3(0.0, 0.0, 1.0);
  vec2 v91 = v6 / screenSize.xy;
  float v92 = texture(depthTex, v91).r;
  float v93 = abs(sceneViewDepth(v6, v2));
  float v94 = abs(v5.z);
  float v95 = (v92 <= 0.0001) ? 100000.0 : (v93 - v94);
  float v96 = v95 / max(v94, 0.0001);
  vec3 v97;
  float v102 = v18.w;
  vec2 v103 = texture(v12, v79.xy).xy + texture(v12, v79.zw).xy - 1.0;
  v103 = v103 * 2.0;
  vec2 v104 = texture(v12, v81.xy).xy + texture(v12, v81.zw).xy - 1.0;
  v104 = v104 * 2.0;
  vec3 v105 = vec3(0.7 * v104 + (v103 - 0.7 * v104) * min(max(0.2 + v88 / 1000.0, 0.0), 1.0), 1.0);
  v105.xy = v105.xy * v102;
  v97 = v105 / sqrt(v105.x * v105.x + v105.y * v105.y + v105.z * v105.z);
  v90.xy = v90.xy + v97.xy;
  v90 = v90 / sqrt(v90.x * v90.x + v90.y * v90.y + v90.z * v90.z);
  float v108;
  vec3 v109 = -v89;
  float v111 = 1.0 - min(max(v109.x * v90.x + v109.y * v90.y + v109.z * v90.z, 0.0), 1.0);
  v108 = v111 * v111;
  vec4 v113 = texture(v10, v82 + abs(0.03 * v97.xy));
  vec4 v114 = texture(v11, v83 + abs(0.03 * v97.xy));
  vec3 v115 = v113.rgb * v113.a * v114.rgb * v114.a * 4.0;
  v115 = v115 + v21.rgb;
  vec2 v116 = v91 + 0.1 * v90.xy * min(max(100.0 * v96 * (1.2 - v108), 0.0), 1.0);
  float v117 = abs(sceneViewDepth(v116 * screenSize.xy, v2));
  bool v118 = v117 > v94;
  v116 = v118 ? v116 : v91;
  vec3 v119 = texture(liquidBackBuffer, v116).rgb;
  float v120 = v118 ? (v117 - v94) : v95;
  vec4 v123 = underWaterFog;
  vec4 v124 = underWaterClassicFogParams;
  float v125;
  if (v124.x != 0.0)
  {
    float v126 = max(0.0, (v124.y - v120) * v124.z);
    v125 = min(max(pow(v126, v124.w), 0.0), 1.0);
  }
  else
  {
    float v127 = max(0.0, (v120 - v123.w) - v123.x);
    float v128 = 1.0 / exp(v127 * v123.z);
    float v129 = v120 / v123.y;
    float v130 = min(max((1.0 / 0.7) * (1.0 - v129), 0.0), 1.0);
    v125 = min(v128, v130);
  }
  vec3 v132 = underWaterFogColor.rgb + (v119 - underWaterFogColor.rgb) * v125;
  v132 = v132 + (v115 - v132) * 0.5;
  float v134 = min(max((v95 - 0.0) / (1.2 * min(max(v108 * 0.5 + 0.2, 0.0), 1.0) - 0.0), 0.0f), 1.0f) * min(max((v95 - 0.0) / (1.2 * min(max(v108 * 0.5 + 0.2, 0.0), 1.0) - 0.0), 0.0f), 1.0f) * (3.0f - 2.0f * min(max((v95 - 0.0) / (1.2 * min(max(v108 * 0.5 + 0.2, 0.0), 1.0) - 0.0), 0.0f), 1.0f));
  float v135 = 1.0 - min(max((v95 * (1.0 - v89.z) - 0.0) / (2.5 * min(max(v108 * 0.5 + 0.2, 0.0), 1.0) - 0.0), 0.0f), 1.0f) * min(max((v95 * (1.0 - v89.z) - 0.0) / (2.5 * min(max(v108 * 0.5 + 0.2, 0.0), 1.0) - 0.0), 0.0f), 1.0f) * (3.0f - 2.0f * min(max((v95 * (1.0 - v89.z) - 0.0) / (2.5 * min(max(v108 * 0.5 + 0.2, 0.0), 1.0) - 0.0), 0.0f), 1.0f));
  float v136 = v20.x * v135;
  vec4 v137 = texture(v13, v80.xy - v90.xy);
  v137 = v137 + texture(v13, v80.zw + 0.2 * v90.yx);
  float v138 = (v136 * (1.1 - min(max(v88 / 150.0, 0.0), 1.0))) * v90.z;
  v137.rgb = v138 * v137.rgb;
  v132 = v132 + v137.rgb;
  float v139;
  vec4 v140 = v1.depthTable;
  float v142 = v9 * v9;
  float v143 = v140.x + v140.y * v9;
  float v144 = v142 * v9;
  v143 = v143 + v142 * v140.z + v144 * v140.w;
  v139 = min(max(v143, 0.0), 1.0);
  v139 = 1.0 - (1.0 - v139) * v20.w;
  v139 = v139 * v85;
  float v146 = v134 * v139;
  vec4 v147 = makeFog2(v3, vec4(v132, v146), v2.uViewUpSceneTime.xyz, v5, v2.extLight.uExteriorDirectColorDir.xyz, 2);
  return vec4(v147.rgb, v146);
}

#endif //LIQUID_LEYLINE_MATERIAL_SLANG
