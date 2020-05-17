#version 420

layout(location = 0) in vec2 texCoord;
layout(location = 0) out vec4 out_result;

layout(set=1,binding=5) uniform sampler2D texture0;

void main()
{
    out_result =texture(texture0, texCoord);
//        (texture(texture0, texCoord*0.5 + vec2(0.4999, 0.502)) * 0.125) +
//        (texture(texture0, texCoord*0.5 + vec2(0.5000, 0.502)) * 0.375) +
//        (texture(texture0, texCoord*0.5 + vec2(0.5000, 0.4999)) * 0.375) +
//        (texture(texture0, texCoord*0.5 + vec2(0.4999, 0.4999)) * 0.125);
}
