// Shadow Map shader

#type vertex
#version 450

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normals;
layout (location = 2) in vec2 a_TexCoord;
layout (location = 3) in vec3 a_Tangent;
layout (location = 4) in vec3 a_Bitangent;

layout (location = 0) uniform mat4 u_Transform; 
layout (location = 1) uniform mat4 u_ViewProjection; // should be outside ubo (ortho matrix)

void main()
{
	gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
}

#type fragment
#version 450

layout(location = 0) out vec4 o_Color;

void main()
{
    o_Color = vec4(vec3(0.3, 0.1, 0.76), 1.0);
}