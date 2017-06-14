#ifdef COMPILING_VS
/* vertex shader code */
attribute vec3 aPosition;

uniform mat4 uLookAtMat;
uniform mat4 uPMatrix;

uniform mat4 uInverseViewProjection;

void main() {

    vec4 c_world = uInverseViewProjection * vec4(aPosition, 1);
    c_world = c_world*1.0/c_world.w;

    gl_Position = uPMatrix * uLookAtMat * vec4(c_world.xyz, 1);
}
#endif //COMPILING_VS

#ifdef COMPILING_FS
precision lowp float;

uniform vec3 uColor;

void main() {
    vec4 finalColor = vec4(1.0, 1.0, 1.0, 1.0);

    finalColor.a = 1.0; //do I really need it now?
    gl_FragColor = finalColor;
}

#endif //COMPILING_FS
