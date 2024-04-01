#include "../../common/commonUboSceneData.glsl"

layout (location = 0) in vec2 position;

layout(std430, set=1, binding=0) buffer readonly pointLightBuffer {
    LocalLight lights[];
};

void main() {

    LocalLight lightRec = lights[gl_InstanceIndex];

    vec4 lightPosView = scene.uLookAtMat * vec4(lightRec.position.xyz, 1.0);

    float attentuationEnd = lightRec.attenuation.z;

    //Hack to get square bounding the circle of point light
    //The quads (-1.0, 1.0) or other is multiplied by radius end of point light
    vec2 quadPosView = (position.xy * attentuationEnd);

    lightPosView.xy += quadPosView;

    //Now it's safe to multiply it by perspective matrix
    gl_Position = scene.uPMatrix * (lightPosView.xyz, 1.0);
}

