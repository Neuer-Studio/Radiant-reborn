#type vertex

#version 450 core
// Physically Based Rendering
// Copyright (c) 2017-2018 Michał Siejak

// Environment skybox: Vertex program.
layout(std140, binding=0) uniform TransformUniforms
{
	mat4 u_ViewProjectionMatrix;
	mat4 u_InversedViewProjectionMatrix;
	mat4 u_ViewMatrix;
};

layout(location=0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord; // we are using the same pipeline with SceneComposite
layout(location=0) out vec3 v_Position;
void main()
{
	vec4 position = vec4(a_Position.xy, 1.0, 1.0);
	gl_Position = position;

	v_Position = (u_InversedViewProjectionMatrix * position).xyz;
}

#type fragment
#version 450 core
// Physically Based Rendering
// Copyright (c) 2017-2018 Michał Siejak

// Environment skybox: Fragment program.

layout(location=0) in vec3 v_Position;
layout(location=0) out vec4 finalColor;

layout(binding=0) uniform samplerCube u_EnvTexture;
layout(std140, binding=10) uniform SkyAttributes
{
	float u_TextureLod;
	float u_SkyIntensity;
};

void main()
{
	vec3 color = textureLod(u_EnvTexture, v_Position, u_TextureLod).rgb * u_SkyIntensity;
	finalColor = vec4(color, 1.0);
}