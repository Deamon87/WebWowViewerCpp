#ifdef COMPILING_VS
precision highp float;
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec4 aColor;
layout(location = 2) in vec2 aTexcoord0;

layout(std140) uniform sceneWideBlockVSPS {
    mat4 uLookAtMat;
    mat4 uPMatrix;
};

out vec4 vColor;
out vec2 vTexcoord0;

void main() {
    vec4 aPositionVec4 = vec4(aPosition, 1);

    vColor = aColor;
    vTexcoord0 = aTexcoord0;
    gl_Position = uPMatrix * uLookAtMat * aPositionVec4;
}
#endif //COMPILING_VS

#ifdef COMPILING_FS
precision highp float;
in vec4 vColor;
in vec2 vTexcoord0;

uniform sampler2D uTexture;

out vec4 outputColor;

void main() {
    vec4 tex = texture(uTexture, vTexcoord0).rgba;

    outputColor = vec4((vColor.rgb*tex.rgb), tex.a * vColor.a );
}
#endif //COMPILING_FS