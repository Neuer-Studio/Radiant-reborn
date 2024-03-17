#type vertex

#version 450 core
layout(location=0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

struct OutputBlock
{
	vec2 TexCoord;
};
layout (location = 0) out OutputBlock Output;

void main()
{
	vec4 position = vec4(a_Position.xy, 1.0, 1.0);
	Output.TexCoord = a_TexCoord;
	gl_Position = position;
}

#type fragment
#version 450 core

struct OutputBlock
{
	vec2 TexCoord;
};

layout (location = 0) in OutputBlock Input;

layout(location=0) out vec4 o_Color;

layout(binding=1) uniform sampler2DMS u_Texture;

layout(location=0) uniform float u_Exposure;
layout(location=1) uniform uint u_SamplesCount;

vec4 MultiSampleTexture(sampler2DMS texture, ivec2 texCoords, uint samples)
{
    vec4 result = vec4(0.0);

    for(int i = 0; i < samples; i++)
        result += texelFetch(texture, texCoords, i);

    result /= float(samples);
    return result;   
}

void main()
{
    ivec2 texSize = textureSize(u_Texture);
	ivec2 texCoord = ivec2(Input.TexCoord * texSize);
	vec4 msColor = MultiSampleTexture(u_Texture, texCoord, u_SamplesCount);

    const float gamma = 2.2;
    const float pureWhite = 1.0;

    vec3 color = msColor.rgb * u_Exposure;
    float luminance = dot(color, vec3(0.2126, 0.7152, 0.0722));
    float mappedLuminance = (luminance * (1.0 + luminance / (pureWhite * pureWhite) ) ) / (1.0 + luminance);
    
    vec3 mappedColor = (mappedLuminance / luminance) * color;

    o_Color = vec4(mappedColor, 1.0);
}
