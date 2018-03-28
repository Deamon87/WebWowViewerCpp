#ifdef COMPILING_VS
/* vertex shader code */
attribute vec2 aPosition;

uniform mat4 uLookAtMat;
uniform mat4 uPMatrix;

uniform mat4 uInverseViewProjection;

void main() {
    gl_Position = uPMatrix * uLookAtMat * vec4(aPosition.xy, 0, 1);
}
#endif //COMPILING_VS

#ifdef COMPILING_FS
precision highp float;

uniform vec3 uColor;

void main() {
    vec4 finalColor = vec4(1.0, 1.0, 0.0, 1.0);

    gl_FragColor = finalColor;
}

#endif //COMPILING_FS
