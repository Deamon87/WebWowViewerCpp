#ifdef COMPILING_VS
layout(location=0) in float aDepth;
layout(location=1) in vec2 aTexCoord;
layout(location=2) in vec3 aPosition;


layout(std140) uniform sceneWideBlockVSPS {
    mat4 uLookAtMat;
    mat4 uPMatrix;
};

layout(std140) uniform modelWideBlockVS {
    mat4 uPlacementMat;
};


out vec2 vTexCoord;
out vec3 vPosition;

void main() {
    mat4 rotateMat = mat4(
        0, 1, 0, 0,
        0, 0, -1, 0,
        1, 0, 0, 0,
        0, 0, 0, 1
    );

   vec4 aPositionVec4 = vec4(aPosition, 1);
   mat4 cameraMatrix = uLookAtMat * uPlacementMat * rotateMat;

   vec4 cameraPoint = cameraMatrix * aPositionVec4;

   gl_Position = uPMatrix * cameraPoint;
   vTexCoord = aTexCoord;
   vPosition = cameraPoint.xyz;
}
#endif

#ifdef COMPILING_FS
in vec2 vTexCoord;
in vec3 vPosition;

uniform sampler2D uTexture;

out vec4 outputColor;

layout(std140) uniform sceneWideBlockVSPS {
    mat4 uLookAtMat;
    mat4 uPMatrix;
};

void main() {
    outputColor = vec4(0.5, 0, 0, 0.5);
}

#endif