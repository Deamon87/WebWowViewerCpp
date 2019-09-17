#version 450

layout(location=0) in vec3 aPosition;
//layout(location=1) in float aDepth;
//layout(location=2) in vec2 aTexCoord;


layout(std140, binding=0) uniform sceneWideBlockVSPS {
    mat4 uLookAtMat;
    mat4 uPMatrix;
};

layout(std140, binding=1) uniform modelWideBlockVS {
    mat4 uPlacementMat;
};


//out vec2 vTexCoord;
layout(location=0) out vec3 vPosition;

void main() {
   vec4 aPositionVec4 = vec4(aPosition, 1);
   mat4 cameraMatrix = uLookAtMat * uPlacementMat ;

   vec4 cameraPoint = cameraMatrix * aPositionVec4;

   gl_Position = uPMatrix * cameraPoint;
   gl_Position.y = -gl_Position.y;
//   vTexCoord = aTexCoord;
   vPosition = cameraPoint.xyz;
}
