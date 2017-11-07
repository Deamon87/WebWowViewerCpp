
#ifdef COMPILING_VS
/* vertex shader code */

attribute vec3 uPos;

varying vec4 vPos;

uniform mat4 uLookAtMat;
uniform mat4 uPMatrix;
uniform mat4 uPMatrix;



void main() {
    gl_Position = uPMatrix * uLookAtMat * vec4(uPos.xyz, 1.0);
    gl_PointSize = 2.0;
}

#endif

#ifdef COMPILING_FS
precision lowp float;

varying vec4 vPos;

uniform vec3 uColor;

void main() {
    gl_FragColor = vec4(color.rgb, 1.0);
}

#endif