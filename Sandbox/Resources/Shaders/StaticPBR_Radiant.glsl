#type vertex
#version 450 core
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normals;
layout (location = 2) in vec2 a_TexCoord;
layout (location = 3) in vec3 a_Tangent;
layout (location = 4) in vec3 a_Bitangent;

layout (std140, binding = 0) uniform Camera
{
   mat4 u_ViewProjectionMatrix;
   mat4 u_InversedViewProjectionMatrix;
};

layout (std140, binding = 1) uniform ModelTransform 
{
	mat4 u_Transform;
};

layout(location=0) out Vertex
{
	vec3 WorldPosition;
	vec2 TexCoord;
	mat3 TangentBasis;
} vout;

void main()
{
      vout.WorldPosition = vec3(u_Transform * vec4(a_Position, 1.0));
      vout.TexCoord = a_TexCoord;//vec2(a_TexCoord.x, 1.0-a_TexCoord.y);

      // Pass tangent space basis vectors (for normal mapping).
      vout.TangentBasis = mat3(u_Transform) * mat3(a_Tangent, a_Bitangent, a_Normals);

     gl_Position = u_ViewProjectionMatrix * u_Transform * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core
layout(location = 0) out vec4 FragColor;

layout(location=0) in Vertex
{
	vec3 WorldPosition;
	vec2 TexCoord;
	mat3 TangentBasis;
} vin;

layout(binding=0) uniform sampler2D u_DiffuseTexture;
layout (location = 13) uniform bool u_DiffuseTextureEnabled; 

void main()
{
   vec3 diffuse = u_DiffuseTextureEnabled ? texture(u_DiffuseTexture, vin.TexCoord).rgb : vec3(0.1, 0.3, 0.5);
   FragColor = vec4(diffuse, 1.0);
}