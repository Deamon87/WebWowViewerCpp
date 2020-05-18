#version 420

layout(location = 0) in vec2 texCoord;
layout(location = 0) out vec4 out_result;

layout(set=1,binding=5) uniform sampler2D texture0;

layout(std140, binding=4) uniform meshWideBlockPS {
    vec4 textureDims;

};

const float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main()
{
    // receive the soze of one texel
    vec2 tex_offset = 1.0 / textureSize(texture0, 0);
    // the value of one fragment
    vec3 result = texture(texture0, texCoord).rgb * weight[0];
    if(textureDims.x > 0)
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(texture0, texCoord + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
            result += texture(texture0, texCoord - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
        }
    }
    else
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(texture0, texCoord + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
            result += texture(texture0, texCoord - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
        }
    }
    out_result = vec4(result, 1.0);

//     out_result =texture(texture0, texCoord);
    //        (texture(texture0, texCoord*0.5 + vec2(0.4999, 0.502)) * 0.125) +
    //        (texture(texture0, texCoord*0.5 + vec2(0.5000, 0.502)) * 0.375) +
    //        (texture(texture0, texCoord*0.5 + vec2(0.5000, 0.4999)) * 0.375) +
    //        (texture(texture0, texCoord*0.5 + vec2(0.4999, 0.4999)) * 0.125);
}
