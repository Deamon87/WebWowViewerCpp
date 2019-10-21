#ifdef COMPILING_VS
layout(location=0) in vec3 aPosition;
//layout(location=1) in float aDepth;
//layout(location=2) in vec2 aTexCoord;


layout(std140) uniform sceneWideBlockVSPS {
    mat4 uLookAtMat;
    mat4 uPMatrix;
};

layout(std140) uniform modelWideBlockVS {
    mat4 uPlacementMat;
};


//out vec2 vTexCoord;
out vec3 vPosition;

precision highp FLOATDEC
void main() {
   vec4 aPositionVec4 = vec4(aPosition, 1);
   mat4 cameraMatrix = uLookAtMat * uPlacementMat ;

   vec4 cameraPoint = cameraMatrix * aPositionVec4;

   gl_Position = uPMatrix * cameraPoint;
//   vTexCoord = aTexCoord;
   vPosition = cameraPoint.xyz;
}
#endif

#ifdef COMPILING_FS
precision highp FLOATDEC

in vec3 vPosition;

uniform sampler2D uTexture;

out vec4 outputColor;

layout(std140) uniform sceneWideBlockVSPS {
    mat4 uLookAtMat;
    mat4 uPMatrix;
};

//Individual meshes
layout(std140) uniform meshWideBlockPS {
    int waterType;
};

void main() {
    if (waterType == 13) { // LIQUID_WMO_Water
        outputColor = vec4(0.0, 0, 0.3, 0.5);
    } else if (waterType == 14) { //LIQUID_WMO_Ocean
        outputColor = vec4(0, 0, 0.8, 0.8);
    } else if (waterType == 19) { //LIQUID_WMO_Magma
        outputColor = vec4(0.3, 0, 0, 0.5);
    } else if (waterType == 20) { //LIQUID_WMO_Slime
        outputColor = vec4(0.0, 0.5, 0, 0.5);
    } else {
        outputColor = vec4(0.5, 0.5, 0.5, 0.5);
    }


}

#endif