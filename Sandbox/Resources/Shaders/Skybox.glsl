#type vertex

#version 450 core
// Physically Based Rendering
// Copyright (c) 2017-2018 Michał Siejak

#include "UBO/Transformations.glsl_h"

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

#include "UBO/Environment/EnvironmentAttributes.glsl_h"

// Physically Based Rendering
// Copyright (c) 2017-2018 Michał Siejak

// Environment skybox: Fragment program.

layout(location=0) in vec3 v_Position;
layout(location=0) out vec4 finalColor;

layout(binding=0) uniform samplerCube u_EnvTexture;

void main()
{
	vec3 color = textureLod(u_EnvTexture, v_Position, TextureLod).rgb * SkyIntensity;
	finalColor = vec4(color, 1.0);
}