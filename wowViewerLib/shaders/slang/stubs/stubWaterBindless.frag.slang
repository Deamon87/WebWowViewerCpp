#version 450

import glsl;

#extension GL_GOOGLE_include_directive: require

precision highp float;
precision highp int;

#include "../common/commonLiquidIndirectDescriptorSet.slang"

/*
-----------------------------------------------------------------------------------
  This is stub shader. It's only purpose is to provide source for Descriptor Layout
-----------------------------------------------------------------------------------
*/

layout(location = 0) out vec4 outColor;

[shader("pixel")]
void main() {
    outColor = vec4(
        uPlacementMats[0][0].x+
        liquidBindlesses[0].liquidDataInd_placementMatInd_liquidMaterialType.x+
        waterDatas[0].computedOffset0_computedOffsets1_pad_pad.x+
        magmaDatas[0].f0_f1_f2_f3.x+
        mercuryDatas[0].f0_f1_f2_f3.x+
        fogDatas[0].f0_f1_f2_f3.x+
        leyLineDatas[0].f0_f1_f2_f3.x+
        felDatas[0].f0_f1_f2_f3.x+
        swampDatas[0].f0_f1_f2_f3.x+
        azeritheDatas[0].f0_f1_f2_f3.x
    );
}