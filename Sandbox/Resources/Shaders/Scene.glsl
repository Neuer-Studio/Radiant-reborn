#type vertex

#version 450 core
// Physically Based Rendering
// Copyright (c) 2017-2018 Michał Siejak

// Environment skybox: Vertex program.
layout(std140, binding=0) uniform TransformUniforms
{
	mat4 u_ViewProjectionMatrix;
};

layout(location=0) in vec3 a_Position;
layout(location=0) out vec3 v_Position;
void main()
{
	vec4 position = vec4(a_Position.xy, 0.5, 0.5);
	gl_Position = position;

	v_Position = (u_ViewProjectionMatrix * position).xyz;
}

#type fragment
#version 450 core
// Physically Based Rendering
// Copyright (c) 2017-2018 Michał Siejak

// Environment skybox: Fragment program.

layout(location=0) in vec3 v_Position;
layout(location=0) out vec4 color;

layout(binding=0) uniform samplerCube u_EnvTexture;

void main()
{
	color = textureLod(u_EnvTexture, v_Position, 0);
}