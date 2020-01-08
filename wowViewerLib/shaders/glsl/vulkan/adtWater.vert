#version 450

layout(location=0) in vec4 aPositionTransp;
layout(location=1) in vec2 aTexCoord;


layout(std140, binding=0) uniform sceneWideBlockVSPS {
    mat4 uLookAtMat;
    mat4 uPMatrix;
};

//out vec2 vTexCoord;
layout(location=0) out vec3 vPosition;

void main() {
   vec4 aPositionVec4 = vec4(aPositionTransp.xyz, 1);
   mat4 cameraMatrix = uLookAtMat;

   vec4 cameraPoint = cameraMatrix * aPositionVec4;

   gl_Position = uPMatrix * cameraPoint;
//   vTexCoord = aTexCoord;
   vPosition = cameraPoint.xyz;
}
