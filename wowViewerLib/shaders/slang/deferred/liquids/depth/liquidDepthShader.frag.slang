#version 450

import glsl;

precision highp float;
precision highp int;

// Depth-only liquid pass: the render pass has no color attachments and depth
// is written by the fixed-function pipeline, so the fragment stage is empty.
[shader("pixel")]
void main() {
}
