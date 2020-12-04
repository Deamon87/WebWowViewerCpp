#version 450

#extension GL_GOOGLE_include_directive: require



layout(location = 0) in vec4 vColor;
layout(location = 0) out vec4 outputColor;

void main() {
//	if (vColor.a < 1.0)
//		discard;

	outputColor = vec4(vColor.xyz, vColor.a);
}
