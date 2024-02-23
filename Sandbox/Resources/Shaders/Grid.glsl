// Grid Shader

#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

layout (std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjectionMatrix;
	mat4 u_InversedViewProjectionMatrix;
};

layout(location = 0) uniform mat4 u_Transform;

layout (location = 0) out vec2 v_TexCoord;

void main()
{
	vec4 position = u_ViewProjectionMatrix * u_Transform * vec4(a_Position, 1.0);
	gl_Position = position;

	v_TexCoord = a_TexCoord;
}

#type fragment
#version 450 core

layout(location = 0) out vec4 color;

layout (location = 0) in vec2 v_TexCoord;

float grid(vec2 st, float res)
{
	vec2 grid = fract(st);
	return step(res, grid.x) * step(res, grid.y);
}

void main()
{
	float x = grid(v_TexCoord * 16.025, 0.07);
	color = vec4(vec3(1.0, 1.0, 1.0), 0.5) * (1.0 - x);
}