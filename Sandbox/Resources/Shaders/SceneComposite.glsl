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

layout(std140, binding=0) uniform Uniforms
{
	float Exposure;
};

layout(binding=1) uniform sampler2D u_Texture;

void main()
{
    const float gamma = 2.2;
    const float pureWhite = 1.0;

    vec3 color = texture(u_Texture, Input.TexCoord).rgb * 1.0;
    float luminance = dot(color, vec3(0.2126, 0.7152, 0.0722));
    float mappedLuminance = (luminance * (1.0 + luminance / (pureWhite * pureWhite) ) ) / (1.0 + luminance);
    
    vec3 mappedColor = (mappedLuminance / luminance) * color;

    o_Color = vec4(mappedColor, 1.0);
}
