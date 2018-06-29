#ifdef COMPILING_VS
/* vertex shader code */
attribute vec3 aPosition;

//Whole scene
uniform mat4 uLookAtMat;
uniform mat4 uPMatrix;
//Whole model
uniform mat4 uPlacementMat;

void main() {
    vec4 worldPoint = vec4(aPosition.xyz, 1);

    gl_Position = uPMatrix * uLookAtMat * uPlacementMat * worldPoint;
}
#endif //COMPILING_VS

#ifdef COMPILING_FS
precision highp float;

//Individual mesh
uniform vec4 uColor;

void main() {
    vec4 finalColor = uColor ;

    gl_FragColor = finalColor;
}

#endif //COMPILING_FS
