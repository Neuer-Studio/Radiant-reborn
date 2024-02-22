#type vertex
#version 450 core
layout (location = 0) in vec3 aPos;
layout(location = 1) in vec2 a_TexCoord;

layout (std140, binding = 0) uniform Camerafff
{
	mat4 u_ViewProjectionMatrix;
};

void main()
{
     gl_Position = u_ViewProjectionMatrix * vec4(aPos, 1.0);
}

#type fragment
#version 450 core
layout(location = 0) out vec4 FragColor;

void main()
{
   FragColor = vec4(vec3(1.0, 1.0, 1.0), 1.0);
}