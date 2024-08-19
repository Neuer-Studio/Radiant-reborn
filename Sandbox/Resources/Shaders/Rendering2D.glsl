#type vertex

#version 450 core

#include "UBO/Transformations.glsl_h"

layout(location=0) in vec3 a_Position;
layout (location = 0) out vec3 o_Color;
void main()
{
	gl_Position = u_ViewProjectionMatrix * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

layout(location=0) out vec4 finalColor;
void main()
{
	finalColor = vec4(1.0);
}