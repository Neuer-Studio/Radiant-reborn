#type vertex

#version 450 core
// Physically Based Rendering
// Copyright (c) 2017-2018 Michał Siejak

// Environment skybox: Vertex program.
layout(std140, binding=0) uniform TransformUniforms
{
	mat4 viewProjectionMatrix;
};

layout(location=0) in vec3 position;
layout(location=0) out vec3 localPosition;

void main()
{
	localPosition = position.xyz;
	gl_Position  = viewProjectionMatrix * vec4(position, 1.0);
}

#type fragment
#version 450 core
// Physically Based Rendering
// Copyright (c) 2017-2018 Michał Siejak

// Environment skybox: Fragment program.

layout(location=0) in vec3 localPosition;
layout(location=0) out vec4 color;

layout(binding=0) uniform samplerCube envTexture;

void main()
{
	vec3 envVector = normalize(localPosition);
	color = textureLod(envTexture, envVector, 0);
}