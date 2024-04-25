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

	mat4 view = u_ViewMatrix;
	view[3][0] = 0;
	view[3][1] = 0;
	view[3][2] = 0; //TODO: move to cpp code

    v_Position = (inverse(u_ProjectionMatrix * view) * position).xyz;
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