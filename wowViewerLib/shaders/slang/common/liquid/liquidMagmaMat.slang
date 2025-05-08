#ifndef LIQUID_MAGMA_MATERIAL_SLANG
#define LIQUID_MAGMA_MATERIAL_SLANG

import glsl;

#include "../commonLiquidStructs.slang"
#include "./commonLiquidFunctions.slang"
#include "../commonLightFunctions.slang"
#include "../commonFogFunctions.slang"

vec4 calcMagmaLiquidMat(in LiquidInstanceData v0, in MagmaData v1, in SceneWideParams v2, in PSFog v3, in vec3 v4, in vec2 v5, in vec2 v6, in float v7, in sampler2D v8, in sampler2D v9, in sampler2D v10, in sampler2D v11)
{
  float v12 = v2.uViewUpSceneTime.w;
  float v13 = v0.flowSpeed_flowDirection_unused4_unused5.x;
  float v14 = v0.flowSpeed_flowDirection_unused4_unused5.y;
  float v17 = v12;
  mat4 v15[6];
  vec4 v16;
  float v21 = v1.f0_f1_f2_f3.x;
  float v22 = v1.f0_f1_f2_f3.y;
  float v23 = v1.f0_f1_f2_f3.z;
  float v24 = v1.f0_f1_f2_f3.w;
  float v25 = v1.f4_f5_f6_f7.x;
  float v26 = v1.f4_f5_f6_f7.y;
  float v27 = v1.f4_f5_f6_f7.z;
  float v28 = v1.f4_f5_f6_f7.w;
  float v29 = v1.f8_f9_f10_f11.x;
  float v30 = v1.f8_f9_f10_f11.y;
  float v31 = v1.f8_f9_f10_f11.z;
  float v32 = v14 * 0.017453292f;
  float v33 = cos(v32);
  float v34 = sin(v32);
  vec2 v35 = vec2((v21 + v13) * v33, (v22 + v13) * v34);
  vec2 v36 = GetTexScrollMtx(v17, v35);
  v15[0] = mat4(vec4(1.0, 0.0, 0.0, 0.0), vec4(0.0, 1.0, 0.0, 0.0), vec4(0.0, 0.0, 1.0, 0.0), vec4(v36.x, v36.y, 0.0, 1.0));
  float v37 = v24 * 0.017453292f;
  float v38 = cos(v37);
  float v39 = sin(v37);
  float v40 = v23 * 0.61699998f;
  vec2 v41 = v35 * 0.5f;
  vec2 v42 = GetTexScrollMtx(v17, v41);
  mat4 v43 = mat4(vec4(v38 * v40, v39 * v40, 0.0, 0.0), vec4(-v39 * v40, v38 * v40, 0.0, 0.0), vec4(0.0, 0.0, v40, 0.0), vec4(0.0, 0.0, 0.0, 1.0));
  mat4 v44 = mat4(vec4(1.0, 0.0, 0.0, 0.0), vec4(0.0, 1.0, 0.0, 0.0), vec4(0.0, 0.0, 1.0, 0.0), vec4(v42.x, v42.y, 0.0, 1.0));
  v15[1] = mul(v44, v43);
  float v45 = v23 * 0.47400001f;
  mat4 v46 = mat4(vec4(v38 * v45, v39 * v45, 0.0, 0.0), vec4(-v39 * v45, v38 * v45, 0.0, 0.0), vec4(0.0, 0.0, v45, 0.0), vec4(0.0, 0.0, 0.0, 1.0));
  v15[2] = mul(v44, v46);
  float v47[3] = {v28 * 0.017453292f, v29 * 0.017453292f, v30 * 0.017453292f};
  float v48[3] = {v25, v26, v27};
  for (int v49 = 0; v49 < 3; ++v49)
  {
    float v50 = cos(v47[v49]);
    float v51 = sin(v47[v49]);
    float v52 = v48[v49];
    mat4 v53 = mat4(vec4(v50 * v52, v51 * v52, 0.0, 0.0), vec4(-v51 * v52, v50 * v52, 0.0, 0.0), vec4(0.0, 0.0, v52, 0.0), vec4(0.0, 0.0, 0.0, 1.0));
    v15[3 + v49] = mul(v15[0], v53);
  }
  v16 = vec4(v31, 0.0, 0.0, mod(v17, 600000.0f) * 0.0000016666667f);
  vec4 v18;
  v18 = vec4(v1.colorRGB_pad.x, v1.colorRGB_pad.y, v1.colorRGB_pad.z, mod(v12, 600000.0f) * 0.0000016666667f);
  vec2 v60 = (v15[0] * vec4(v5, 0.0, 1.0)).xy;
  float v61 = (cos(v60.x) + sin(v60.y)) * 0.5;
  float v62 = (cos(v60.x * 2.0) + sin(v60.y * 2.0)) * 0.5;
  float v63 = (cos(v60.x * 3.0) + sin(v60.y * 3.0)) * 0.5;
  float v64 = (v61 + v62 + v63) * 0.3 * 0.5 + 0.5;
  vec2 v65 = (v15[1] * vec4(v5, 0.0, 1.0)).xy;
  vec2 v66 = (v15[2] * vec4(v5, 0.0, 1.0)).xy;
  vec2 v67 = (v15[3] * vec4(v5, 0.0, 1.0)).xy;
  vec2 v68 = (v15[4] * vec4(v5, 0.0, 1.0)).xy;
  vec2 v69 = (v15[5] * vec4(v5, 0.0, 1.0)).xy;
  float v72 = v18.w;
  vec3 v73;
  float v77 = v72 * 16.0;
  float v78 = floor(v77);
  float v79 = v77 - v78;
  float v80 = mod(v78 + 1.0, 16.0);
  vec3 v81 = texture(v11, vec2(v65.x, (v65.y + v78) * 0.0625)).rgb;
  vec3 v82 = texture(v11, vec2(v65.x, (v65.y + v80) * 0.0625)).rgb;
  v73 = v81 + (v82 - v81) * v79;
  vec3 v84;
  float v88 = v72 * 16.0;
  float v89 = floor(v88);
  float v90 = v88 - v89;
  float v91 = mod(v89 + 1.0, 16.0);
  vec3 v92 = texture(v11, vec2(v66.x, (v66.y + v89) * 0.0625)).rgb;
  vec3 v93 = texture(v11, vec2(v66.x, (v66.y + v91) * 0.0625)).rgb;
  v84 = v92 + (v93 - v92) * v90;
  vec3 v95 = v73 + (v84 - v73) * v64;
  vec4 v96 = texture(v8, v67);
  vec4 v97 = texture(v9, v68);
  vec4 v98 = texture(v10, v69);
  vec3 v99 = v18.rgb;
  v99 = v99 + (v96.rgb - v99) * v95.x;
  v99 = v99 + (v97.rgb - v99) * (v97.a * v95.y);
  v99 = v99 + (v98.rgb - v99) * (v98.a * v95.z);
  float v100 = 1.0f;
  if (v100 < 0.1)
    discard;
  vec4 v101 = makeFog2(v3, vec4(v99, v100), v2.uViewUpSceneTime.xyz, v4, v2.extLight.uExteriorDirectColorDir.xyz, 2);
  return vec4(v101.rgb, v100);
}

#endif //LIQUID_MAGMA_MATERIAL_SLANG
