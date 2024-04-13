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

layout (std140, binding = 0) uniform TransformUniforms
{
   mat4 u_ViewProjectionMatrix;
   mat4 u_InversedViewProjectionMatrix; 
   mat4 u_ViewMatrix;
};

layout (location = 0) uniform mat4 u_Transform;

// Shadows
layout (location = 1) uniform mat4 u_LightMatrixCascade;

layout(location=0) out VertexOutput
{
	vec3 WorldPosition;
    vec3 Normal;
	vec2 TexCoord;
	mat3 WorldNormals;
	mat3 WorldTransform;
	vec3 Binormal;
	vec4 ShadowMapCoords;
	vec3 ViewPosition;
} vs_Output;

void main()
{
    vs_Output.WorldPosition = vec3(u_Transform * vec4(a_Position, 1.0));
    vs_Output.TexCoord = vec2(a_TexCoord.x, 1.0-a_TexCoord.y);
   	vs_Output.Normal = mat3(u_Transform) * a_Normals;
    // Pass tangent space basis vectors (for normal mapping).
    vs_Output.WorldNormals = mat3(u_Transform) * mat3(a_Tangent, a_Bitangent, a_Normals);
  	vs_Output.ViewPosition = vec3(u_ViewMatrix * vec4(vs_Output.WorldPosition, 1.0));
	vs_Output.WorldTransform = mat3(u_Transform);
	vs_Output.Binormal = a_Bitangent;
    vs_Output.ShadowMapCoords = u_LightMatrixCascade * vec4(vs_Output.WorldPosition, 1.0);

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
	mat3 WorldTransform;
	vec3 Binormal;
	vec4 ShadowMapCoords;
	vec3 ViewPosition;
} vs_Input;

// PBR texture inputs
layout(binding= 0) uniform sampler2D u_AlbedoTexture;
layout(binding= 1) uniform sampler2D u_NormalTexture;
layout(binding= 2) uniform sampler2D u_MetalnessTexture;
layout(binding= 3) uniform sampler2D u_RoughnessTexture;
// Environment maps
layout(binding= 4) uniform samplerCube u_EnvRadianceTex;
layout(binding= 5) uniform samplerCube u_EnvIrradianceTex;

// Shadows
layout(binding = 10) uniform sampler2D u_ShadowMapTexture;

// BRDF LUT
layout(binding = 3) uniform sampler2D u_BRDFLUTTexture;

layout (location = 2) uniform vec3 u_AlbedoColor; // u_Transform is already using binding =  0
layout (location = 3) uniform float u_Metalness; // u_LightView is already using binding =  1
layout (location = 4) uniform float u_Roughness;

layout(location = 5) uniform bool u_UseAlbedoTexture;
layout(location = 6) uniform bool u_UseNormalTexture;
layout(location = 7) uniform bool u_UseMetalnessTexture;
layout(location = 8) uniform bool u_UseRoughnessTexture;

struct EnvironmentLight 
{
	vec3 Direction;
	vec3 Radiance;

    float Multiplier;
};

layout(std140, binding=2) uniform ShadingUniforms
{
    EnvironmentLight u_EnvironmentLight[NUM_LIGHTS]; 
    vec3 u_CameraPosition;
    float u_EnvMapRotation;
    bool u_IBLContribution;
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

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// Shlick's approximation of the Fresnel factor.
vec3 fresnelSchlick(vec3 F0, float cosTheta)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 fresnelSchlickRoughness(vec3 F0, float cosTheta, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
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

// ---------------------------------------------------------------------------------------------------

vec3 RotateVectorAboutY(float angle, vec3 vec)
{
    angle = radians(angle);
    mat3x3 rotationMatrix ={vec3(cos(angle),0.0,sin(angle)),
                            vec3(0.0,1.0,0.0),
                            vec3(-sin(angle),0.0,cos(angle))};
    return rotationMatrix * vec;
}

vec3 Lighting(vec3 F0)
{
	vec3 result = vec3(0.0);

    for(int i = 0; i < NUM_LIGHTS; i++)
    {
        vec3 Li = u_EnvironmentLight[i].Direction;
        vec3 Lradiance = u_EnvironmentLight[i].Radiance * u_EnvironmentLight[i].Multiplier;
        vec3 Lh = normalize(Li + m_Params.View);

        // Calculate angles between surface normal and various light vectors.
        float cosLi = max(0.0, dot(m_Params.Normal, Li));
        float cosLh = max(0.0, dot(m_Params.Normal, Lh));

        vec3 F = fresnelSchlick(F0, max(0.0, dot(Lh, m_Params.View)));
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

//////////////////
// Shadows ///////
//////////////////

float ShadowFade = 1.0;

float GetShadowBias(vec3 direction)
{
	const float MINIMUM_SHADOW_BIAS = 0.002;
	float bias = max(MINIMUM_SHADOW_BIAS * (1.0 - dot(m_Params.Normal, direction)), MINIMUM_SHADOW_BIAS);
	return bias;
}

float HardShadows_DirectionalLight(sampler2D shadowMap, vec3 shadowCoords, vec3 lightDirection)
{
	float bias = GetShadowBias(lightDirection);
	float shadowMapDepth = texture(shadowMap, shadowCoords.xy * 0.5 + 0.5).x;

    return step(shadowCoords.z, shadowMapDepth + bias) * ShadowFade;
}

// float PCSS_DirectionalLight(sampler2D shadowMap, vec3 shadowCoords, float uvLightSize)
// {
// 	float blockerDistance = FindBlockerDistance_DirectionalLight(shadowMap, shadowCoords, uvLightSize);
// 	if (blockerDistance == -1) // No occlusion
// 		return 1.0f;

// 	float penumbraWidth = (shadowCoords.z - blockerDistance) / blockerDistance;

// 	float NEAR = 0.01; // Should this value be tweakable?
// 	float uvRadius = penumbraWidth * uvLightSize * NEAR / shadowCoords.z; // Do we need to divide by shadowCoords.z?
// 	uvRadius = min(uvRadius, 0.002f);
// 	return PCF_DirectionalLight(shadowMap, shadowCoords, uvRadius) * ShadowFade;
// }

// float FindBlockerDistance_DirectionalLight(sampler2D shadowMap, vec3 shadowCoords, float uvLightSize, vec3 lightDirection)
// {
// 	float bias = GetShadowBias(lightDirection);

// 	int numBlockerSearchSamples = 64;
// 	int blockers = 0;
// 	float avgBlockerDistance = 0;

// 	float searchWidth = SearchRegionRadiusUV(shadowCoords.z);
// 	for (int i = 0; i < numBlockerSearchSamples; i++)
// 	{
// 		float z = textureLod(shadowMap, (shadowCoords.xy * 0.5 + 0.5) + SamplePoisson(i) * searchWidth, 0).r;
// 		if (z < (shadowCoords.z - bias))
// 		{
// 			blockers++;
// 			avgBlockerDistance += z;
// 		}
// 	}

// 	if (blockers > 0)
// 		return avgBlockerDistance / float(blockers);

// 	return -1;
// }

//////////////////
/////////////////
////////////////

vec3 IBL(vec3 F0, vec3 Lr)
{
    vec3 irradiance = texture(u_EnvIrradianceTex, m_Params.Normal).rgb;
    vec3 F = fresnelSchlickRoughness(F0, m_Params.NdotV, m_Params.Roughness);
    vec3 kd = (1.0 - F) * (1.0 - m_Params.Metalness);
    vec3 diffuseIBL = m_Params.Albedo * irradiance;

    int EnvRadianceTexLevels = textureQueryLevels(u_EnvRadianceTex); 
    float NoV = clamp(m_Params.NdotV, 0.0, 1.0);
    vec3 R = 2.0 * dot(m_Params.View, m_Params.Normal) * m_Params.Normal * m_Params.View;
    vec3 specularIrradiance = textureLod(u_EnvRadianceTex, RotateVectorAboutY(u_EnvMapRotation, Lr), (m_Params.Roughness) * EnvRadianceTexLevels).rgb;

    vec2 specularBRDF = texture(u_BRDFLUTTexture, vec2(m_Params.NdotV, 1.0 - m_Params.Roughness)).rg;
    vec3 specularIBL  = specularIrradiance * ( F * specularBRDF.x + specularBRDF.y);

    return kd * diffuseIBL + specularIBL;
}

void main()
{
    m_Params.Normal = normalize(vs_Input.Normal);
    if(u_UseNormalTexture)
    {
        m_Params.Normal = normalize(texture(u_NormalTexture, vs_Input.TexCoord).rgb * 2.0 - 1.0);
        m_Params.Normal = normalize(vs_Input.WorldNormals * m_Params.Normal);
    }
        
 	m_Params.View = normalize(u_CameraPosition - vs_Input.WorldPosition);
    m_Params.NdotV = max(0.0, dot(m_Params.Normal, m_Params.View));

    m_Params.Albedo = u_UseAlbedoTexture ?  texture(u_AlbedoTexture, vs_Input.TexCoord).rgb
                                            : u_AlbedoColor;

    m_Params.Metalness = u_UseMetalnessTexture ? texture(u_MetalnessTexture, vs_Input.TexCoord).r
                                                : u_Metalness;

    m_Params.Roughness = u_UseRoughnessTexture ? texture(u_RoughnessTexture, vs_Input.TexCoord).r
                                                : u_Roughness;
    //m_Params.Roughness = max(m_Params.Roughness, 0.8);

	vec3 Lr = 2.0 * m_Params.NdotV * m_Params.Normal - m_Params.View;

	// Fresnel reflectance, metals use albedo
	vec3 F0 = mix(Fdielectric, m_Params.Albedo, m_Params.Metalness);

	float NdotL = dot(m_Params.Normal, u_EnvironmentLight[0].Direction);

    vec3 shadowMapCoords = (vs_Input.ShadowMapCoords.xyz / vs_Input.ShadowMapCoords.w);
    shadowMapCoords.z = shadowMapCoords.z * 0.5 + 0.5; // scale bias for OpenGL depth value
    float shadowAmount = HardShadows_DirectionalLight(u_ShadowMapTexture, shadowMapCoords, u_EnvironmentLight[0].Direction);
    shadowAmount *= (NdotL * 1.0);
    vec3 lightContribution = Lighting(F0) * shadowAmount;

    vec3 iblContribution = IBL(F0, Lr);

    o_Color = vec4(lightContribution + iblContribution * 0.001, 1.0);
}