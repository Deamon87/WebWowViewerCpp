#version 450

#extension GL_GOOGLE_include_directive: require



layout(location = 0) in vec4 vColor;
layout(location = 0) out vec4 outputColor;

void main() {
	outputColor = vec4(vColor.xyz, 1.0);
}
