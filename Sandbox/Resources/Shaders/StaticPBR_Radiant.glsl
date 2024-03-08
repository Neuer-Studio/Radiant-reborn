// Physically Based Rendering
// Copyright (c) 2017-2018 Michał Siejak

// Physically Based shading model: Lambetrtian diffuse BRDF + Cook-Torrance microfacet specular BRDF + IBL for ambient.

// This implementation is based on "Real Shading in Unreal Engine 4" SIGGRAPH 2013 course notes by Epic Games.
// See: http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
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

layout (location = 5) uniform mat4 u_Transform;

layout(location=0) out VertexOutput
{
	vec3 WorldPosition;
	vec3 Normal;
	vec2 TexCoord;
	mat3 WorldNormals;
} vs_Output;

void main()
{
      vs_Output.WorldPosition = vec3(u_Transform * vec4(a_Position, 1.0));
      vs_Output.TexCoord = vec2(a_TexCoord.x, 1.0-a_TexCoord.y);
      vs_Output.Normal = vec3(u_Transform * vec4(a_Normals, 1.0));

      // Pass tangent space basis vectors (for normal mapping).
      vs_Output.WorldNormals = mat3(u_Transform) * mat3(a_Tangent, a_Bitangent, a_Normals);

     gl_Position = u_ViewProjectionMatrix * u_Transform * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core
layout(location = 0) out vec4 o_Color;

const float PI = 3.141592;
const float Epsilon = 0.00001;
const int NUM_LIGHTS = 1;

// Constant normal incidence Fresnel factor for all dielectrics.
const vec3 Fdielectric = vec3(0.04);

layout(location=0) in VertexOutput
{
	vec3 WorldPosition;
	vec3 Normal;
	vec2 TexCoord;
	mat3 WorldNormals;
} vs_Input;

// PBR texture inputs
layout(binding=0) uniform sampler2D u_AlbedoTexture;
layout(binding=1) uniform sampler2D u_NormalTexture;
layout(binding=2) uniform sampler2D u_MetalnessTexture;
layout(binding=3) uniform sampler2D u_RoughnessTexture;
// Environment maps
layout(binding=4) uniform samplerCube u_EnvRadianceTex;
layout(binding=5) uniform samplerCube u_EnvIrradianceTex;
// BRDF LUT
layout(binding=6) uniform sampler2D u_BRDFLUTTexture;

layout (location = 0) uniform vec3 u_AlbedoColor;
layout (location = 1) uniform float u_Metalness;
layout (location = 2) uniform float u_Roughness;

struct EnvironmentLight 
{
	vec3 Direction;
	vec3 Radiance;
};

layout(std140, binding=2) uniform ShadingUniforms
{
    EnvironmentLight u_EnvironmentLight[NUM_LIGHTS]; 
    vec3 u_CameraPosition;
    float u_EnvMapRotation;
    bool u_RadiancePrefilter;
};

struct PBRParams
{
	vec3 Albedo;
	float Roughness;
	float Metalness;

	vec3 Normal;
	vec3 View;
	float NdotV; // NOTE: Angle between surface normal and outgoing light direction.
};

PBRParams m_Params;

// GGX/Towbridge-Reitz normal distribution function.
// Uses Disney's reparametrization of alpha = roughness^2
float ndfGGX(float cosLh, float roughness)
{
	float alpha = roughness * roughness;
	float alphaSq = alpha * alpha;

	float denom = (cosLh * cosLh) * (alphaSq - 1.0) + 1.0;
	return alphaSq / (PI * denom * denom);
}

float DistributionGGX(vec3 N, vec3 H, float a)
{
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float nom    = a2;
    float denom  = (NdotH2 * (a2 - 1.0) + 1.0);
    denom        = PI * denom * denom;
	
    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float k)
{
    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return nom / denom;
}
  
float GeometrySmith(vec3 N, vec3 V, vec3 L, float k)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = GeometrySchlickGGX(NdotV, k);
    float ggx2 = GeometrySchlickGGX(NdotL, k);
	
    return ggx1 * ggx2;
}

// Single term for separable Schlick-GGX below.
float gaSchlickG1(float cosTheta, float k)
{
	return cosTheta / (cosTheta * (1.0 - k) + k);
}

// Schlick-GGX approximation of geometric attenuation function using Smith's method.
float gaSchlickGGX(float cosLi, float NdotV, float roughness)
{
	float r = roughness + 1.0;
	float k = (r * r) / 8.0; // Epic suggests using this roughness remapping for analytic lights.
	return gaSchlickG1(cosLi, k) * gaSchlickG1(NdotV, k);
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
} 

vec3 RotateVectorAboutY(float angle, vec3 vec)
{
    angle = radians(angle);
    mat3 rotationMatrix = 
    {
        vec3(cos(angle), 0.0, sin(angle)),
        vec3(0.0,1.0,0.0),
        vec3(-sin(angle), 0.0, cos(angle))
    };

    return rotationMatrix * vec;
}

// ---------------------------------------------------------------------------------------------------
// The following code (from Unreal Engine 4's paper) shows how to filter the environment map
// for different roughnesses. This is mean to be computed offline and stored in cube map mips,
// so turning this on online will cause poor performance
float RadicalInverse_VdC(uint bits) 
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

vec2 Hammersley(uint i, uint N)
{
    return vec2(float(i)/float(N), RadicalInverse_VdC(i));
}

vec3 ImportanceSampleGGX(vec2 Xi, float Roughness, vec3 N)
{
	float a = Roughness * Roughness;
	float Phi = 2 * PI * Xi.x;
	float CosTheta = sqrt( (1 - Xi.y) / ( 1 + (a*a - 1) * Xi.y ) );
	float SinTheta = sqrt( 1 - CosTheta * CosTheta );
	vec3 H;
	H.x = SinTheta * cos( Phi );
	H.y = SinTheta * sin( Phi );
	H.z = CosTheta;
	vec3 UpVector = abs(N.z) < 0.999 ? vec3(0,0,1) : vec3(1,0,0);
	vec3 TangentX = normalize( cross( UpVector, N ) );
	vec3 TangentY = cross( N, TangentX );
	// Tangent to world space
	return TangentX * H.x + TangentY * H.y + N * H.z;
}

float TotalWeight = 0.0;

vec3 PrefilterEnvMap(float Roughness, vec3 R)
{
	vec3 N = R;
	vec3 V = R;
	vec3 PrefilteredColor = vec3(0.0);
	int NumSamples = 1024;
	for(int i = 0; i < NumSamples; i++)
	{
		vec2 Xi = Hammersley(i, NumSamples);
		vec3 H = ImportanceSampleGGX(Xi, Roughness, N);
		vec3 L = 2 * dot(V, H) * H - V;
		float NoL = clamp(dot(N, L), 0.0, 1.0);
		if (NoL > 0)
		{
			PrefilteredColor += texture(u_EnvRadianceTex, L).rgb * NoL;
			TotalWeight += NoL;
		}
	}
	return PrefilteredColor / TotalWeight;
}

vec3 Lighting(vec3 F0)
{
	vec3 result = vec3(0.0);

    for(int i = 0; i < NUM_LIGHTS; i++)
    {
        vec3 Li = -u_EnvironmentLight[i].Direction;
        vec3 Lradiance = u_EnvironmentLight[i].Radiance;
        vec3 Lh = normalize(Li + m_Params.View);

        // Calculate angles between surface normal and various light vectors.
        float cosLi = max(0.0, dot(m_Params.Normal, Li));
        float cosLh = max(0.0, dot(m_Params.Normal, Lh));

        vec3 F = fresnelSchlick(max(0.0, dot(Lh, m_Params.View)), F0);
        float D = ndfGGX(cosLh, m_Params.Roughness);
        float G = gaSchlickGGX(cosLi, m_Params.NdotV, m_Params.Roughness);

        vec3 kd = (1.0 - F) * (1.0 - m_Params.Metalness);
        vec3 diffuseBRDF = kd * m_Params.Albedo;

        // Cook-Torrance
        vec3 specularBRDF = (F * D * G) / max(Epsilon, 4.0 * cosLi * m_Params.NdotV);
        result += (diffuseBRDF + specularBRDF) * Lradiance + cosLi;
   }

    return result;
}

vec3 IBL(vec3 F0, vec3 Lr)
{
    vec3 irradiance = texture(u_EnvIrradianceTex, m_Params.Normal).rgb;
    vec3 F = fresnelSchlickRoughness(m_Params.NdotV, F0, m_Params.Roughness);
    vec3 kd = (1.0 - F) * (1.0 - m_Params.Metalness);
    vec3 diffuseIBL = m_Params.Albedo * irradiance;

    int EnvRadianceTexLevels = textureQueryLevels(u_EnvRadianceTex); 
    float NoV = clamp(m_Params.NdotV, 0.0, 1.0);
    vec3 R = 2.0 * dot(m_Params.View, m_Params.Normal) * m_Params.Normal * m_Params.View;
    vec3 specularIrradiance = vec3(0.0);

    if(u_RadiancePrefilter)
        specularIrradiance = PrefilterEnvMap(m_Params.Roughness * m_Params.Roughness, R);
    else
        specularIrradiance = textureLod(u_EnvRadianceTex, RotateVectorAboutY(u_EnvMapRotation, Lr), sqrt(m_Params.Roughness) * EnvRadianceTexLevels).rgb;

    vec2 specularBRDF = texture(u_BRDFLUTTexture, vec2(m_Params.NdotV, 1.0 - m_Params.Roughness)).rg;
    vec3 specularIBL  = specularIrradiance * ( F * specularBRDF.x + specularBRDF.y);

    return kd * diffuseIBL + specularIBL;
}

void main()
{
	m_Params.Normal = normalize(vs_Input.Normal);
 	m_Params.View = normalize(u_CameraPosition - vs_Input.WorldPosition);
    m_Params.NdotV = max(0.0, dot(m_Params.Normal, m_Params.View));

    m_Params.Albedo = u_AlbedoColor;
    m_Params.Metalness = u_Metalness;
    m_Params.Roughness = max(u_Roughness, 0.05);

    // Fresnel reflectance, metals use albedo
    vec3 F0 = mix(Fdielectric, m_Params.Albedo, m_Params.Metalness);
    vec3 lightContribution = Lighting(F0);

    vec3 Lr = 2.0 * m_Params.NdotV * m_Params.Normal - m_Params.View;
    vec3 iblContribution = IBL(F0, Lr);

    o_Color = vec4(lightContribution + iblContribution, 1.0);
}