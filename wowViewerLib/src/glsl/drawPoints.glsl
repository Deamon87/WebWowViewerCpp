
#ifdef COMPILING_VS
/* vertex shader code */

attribute vec3 aPosition;

varying vec4 vPos;

uniform mat4 uLookAtMat;
uniform mat4 uPMatrix;
uniform mat4 uPlacementMat;



void main() {
    gl_Position = uPMatrix * uLookAtMat * uPlacementMat * vec4(aPosition.xyz, 1.0);
    gl_PointSize = 10.0;
}

#endif

#ifdef COMPILING_FS
precision highp float;

varying vec4 vPos;

uniform vec3 uColor;

void main() {
    gl_FragColor = vec4(uColor.rgb, 1.0);
}

#endif