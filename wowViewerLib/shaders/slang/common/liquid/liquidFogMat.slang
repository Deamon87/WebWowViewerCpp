#ifndef LIQUID_FOG_MATERIAL_SLANG
#define LIQUID_FOG_MATERIAL_SLANG

import glsl;

#include "../commonLiquidStructs.slang"
#include "./commonLiquidFunctions.slang"
#include "../commonFogFunctions.slang"

vec4 calcLiquidFogMat(in LiquidInstanceData v0, in FogData v1, in SceneWideParams v2, in PSFog v3, in vec3 v4, in vec3 v5, in vec2 v6, in vec2 v7, in vec2 v8, in float v9, in sampler2D v10)
{
  float v11 = v2.uViewUpSceneTime.w;
  mat4 v12[4];
  mat4 v13[4];
  float v16 = v1.f0_f1_f2_f3.w;
  float v17 = v1.f4_f5_f6_f7.x;
  float v18 = v1.f4_f5_f6_f7.y;
  float v19 = v1.f8_f9_f10_f11.y;
  float v20 = v1.f8_f9_f10_f11.z;
  float v21 = v18 * 0.017453292f;
  float v22 = cos(v21);
  float v23 = sin(v21);
  v13[0] = mat4(vec4(v22 * v16, v23 * v16, 0.0, 0.0), vec4(-v23 * v16, v22 * v16, 0.0, 0.0), vec4(0.0, 0.0, v16, 0.0), vec4(0.0, 0.0, 0.0, 1.0));
  v13[1] = mat4(vec4(v22 * v17, v23 * v17, 0.0, 0.0), vec4(-v23 * v17, v22 * v17, 0.0, 0.0), vec4(0.0, 0.0, v17, 0.0), vec4(0.0, 0.0, 0.0, 1.0));
  vec2 v24 = GetTexScrollMtx(v11, vec2(v19, 0.0f));
  v13[2] = mat4(vec4(1.0, 0.0, 0.0, 0.0), vec4(0.0, 1.0, 0.0, 0.0), vec4(0.0, 0.0, 1.0, 0.0), vec4(v24.x, v24.y, 0.0, 1.0));
  vec2 v25 = GetTexScrollMtx(v11, vec2(v20, 0.0f));
  v13[3] = mat4(vec4(1.0, 0.0, 0.0, 0.0), vec4(0.0, 1.0, 0.0, 0.0), vec4(0.0, 0.0, 1.0, 0.0), vec4(v25.x, v25.y, 0.0, 1.0));
  v12[0] = v13[0];
  v12[1] = v13[1];
  v12[2] = v13[2];
  v12[3] = v13[3];
  mat4 v14[4];
  v14[0] = v12[0];
  v14[1] = v12[1];
  v14[2] = v12[2];
  v14[3] = v12[3];
  vec4 v27;
  v27.xy = (v14[0] * vec4(v7, 0.0, 1.0)).xy;
  v27.zw = (v14[1] * vec4(v7 + 0.5, 0.0, 1.0)).xy;
  v27.xy = (v14[2] * vec4(v27.xy, 0.0, 1.0)).xy;
  v27.zw = (v14[3] * vec4(v27.zw, 0.0, 1.0)).xy;
  vec4 v28 = texture(v10, v27.xy);
  vec4 v29 = texture(v10, v27.zw);
  float v31 = (v9 == 0.0) ? 0.0 : 1.0;
  vec3 v32 = (v2.uInvLookAtMat * vec4(0.0, 0.0, 0.0, 1.0)).xyz;
  vec3 v33 = v4 - v32;
  float v34 = abs(sceneViewDepth(v6, v2));
  float v35 = abs(v5.z);
  float v36 = v34 - v35;
  vec3 v37 = v33 / sqrt(v33.x * v33.x + v33.y * v33.y + v33.z * v33.z) * v36;
  float v38;
  float v40 = min(max(v28.a * v29.a + v29.a, 0.0), 1.0);
  vec2 v41 = vec2(v1.f0_f1_f2_f3.x, v1.f0_f1_f2_f3.y);
  float v42 = v32.z;
  float v43 = v41.x;
  float v44 = v41.y * v40;
  float v45 = exp(-v43 * v42);
  float v46 = sqrt(v37.x * v37.x + v37.y * v37.y + v37.z * v37.z) * v45;
  const float v47 = 0.01;
  if (v37.z > v47)
  {
    v46 = min(max(v46, 0.0), v40 * 2.0);
  }
  v38 = min(max(exp(-v44 * v46), 0.0), 1.0);
  float v49 = 1.0 - v38;
  vec3 v50 = v1.fogColor.rgb * v1.fogColor.a;
  v49 = v49 * v31;
  vec4 v51 = makeFog2(v3, vec4(v50, v49), v2.uViewUpSceneTime.xyz, v5, v2.extLight.uExteriorDirectColorDir.xyz, 2);
  return vec4(v51.rgb, v49);
}

#endif //LIQUID_FOG_MATERIAL_SLANG
