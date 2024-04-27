// Physically Based Rendering
// Copyright (c) 2017-2018 Michał Siejak

// Physically Based shading model: Lambetrtian diffuse BRDF + Cook-Torrance microfacet specular BRDF + IBL for ambient.

// This implementation is based on "Real Shading in Unreal Engine 4" SIGGRAPH 2013 course notes by Epic Games.
// See: http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf

#type vertex
#version 450 core

#include "UBO/Transformations.glsl_h"

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normals;
layout (location = 2) in vec2 a_TexCoord;
layout (location = 3) in vec3 a_Tangent;
layout (location = 4) in vec3 a_Bitangent;

layout (location = 0) uniform mat4 u_Transform;

// Shadows
layout (location = 11) uniform mat4 u_LightMatrixCascade[4];

layout(location=0) out VertexOutput
{
	vec3 WorldPosition;
    vec3 Normal;
	vec2 TexCoord;
	mat3 WorldNormals;
	mat3 WorldTransform;
	vec3 Binormal;
	vec4 ShadowMapCoords[4];
	vec3 ViewPosition;
    vec3 CameraPosition;
} vs_Output;

void main()
{
    vs_Output.WorldPosition = vec3(u_Transform * vec4(a_Position, 1.0));
    vs_Output.TexCoord = vec2(a_TexCoord.x, 1.0-a_TexCoord.y);
   	vs_Output.Normal =  mat3(u_Transform) * a_Normals;
    // Pass tangent space basis vectors (for normal mapping).
    vs_Output.WorldNormals = mat3(u_Transform) * mat3(a_Tangent, a_Bitangent, a_Normals);
  	vs_Output.ViewPosition = vec3(u_ViewMatrix * vec4(vs_Output.WorldPosition, 1.0));
	vs_Output.WorldTransform = mat3(u_Transform);
	vs_Output.Binormal = a_Bitangent;
	vs_Output.CameraPosition = u_CameraPosition;

    vs_Output.ShadowMapCoords[0] = u_LightMatrixCascade[0] * vec4(vs_Output.WorldPosition, 1.0);
	vs_Output.ShadowMapCoords[1] = u_LightMatrixCascade[1] * vec4(vs_Output.WorldPosition, 1.0);
	vs_Output.ShadowMapCoords[2] = u_LightMatrixCascade[2] * vec4(vs_Output.WorldPosition, 1.0);
	vs_Output.ShadowMapCoords[3] = u_LightMatrixCascade[3] * vec4(vs_Output.WorldPosition, 1.0);

    gl_Position = u_ViewProjectionMatrix * u_Transform * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core
layout(location = 0) out vec4 o_Color;

#include "UBO/Environment/EnvironmentAttributes.glsl_h"

const float PI = 3.141592;
const float Epsilon = 0.00001;
const int LightCount = 1;

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
	vec4 ShadowMapCoords[4];
	vec3 ViewPosition;
    vec3 CameraPosition;
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
layout(binding = 20) uniform sampler2D u_ShadowMapTexture[4];

layout (location = 16) uniform vec4 u_CascadeSplits;

// BRDF LUT
layout(binding = 3) uniform sampler2D u_BRDFLUTTexture;

layout (location = 1) uniform mat4 u_LightView; // u_Transform is already using binding =  0
layout (location = 2) uniform vec3 u_AlbedoColor; 
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
    float Intensity;

	bool CastShadow;
};

struct PointLight 
{
	vec3 Position;
	vec3 Radiance;
    float Intensity;
	float Radius;
	float Falloff;
	float LightSize;
};

struct PointLightDeclaration 
{
	uint size;
	PointLight pointLights[1024];
};

layout(std140, binding=2) uniform Lights
{
    EnvironmentLight environmentLight[LightCount];
	PointLightDeclaration pointLight;
} u_Light;

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
			//PrefilteredColor += texture(u_EnvRadianceTex, L).rgb * NoL;
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
	for(int i = 0; i < LightCount; i++)
	{
		vec3 Li = u_Light.environmentLight[i].Direction;
		vec3 Lradiance = u_Light.environmentLight[i].Radiance * u_Light.environmentLight[i].Intensity;
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

		result += (diffuseBRDF + specularBRDF) * Lradiance * cosLi;
	}
	return result;
}

vec3 CalculatePointLight(const PointLight light, vec3 worldPos)
{
	vec3 lightDirection = light.Position - worldPos;

	// attenuation
	float distance = length(lightDirection);
	float attenuation = clamp(1.0 - (distance - 1.0) / (light.Radius - 1.0), 0.0, 1.0);
	float falloff = pow(attenuation, light.Falloff); 

	float lightSizeAttenuation = 1.0 - smoothstep(0.0, light.LightSize, distance);
	float intensity = light.Intensity * falloff * lightSizeAttenuation;

	return light.Radiance * intensity;

}

vec3 IBL(vec3 F0, vec3 Lr)
{
	vec3 irradiance = texture(u_EnvIrradianceTex, m_Params.Normal).rgb;
	vec3 F = fresnelSchlickRoughness(F0, m_Params.NdotV, m_Params.Roughness);
	vec3 kd = (1.0 - F) * (1.0 - m_Params.Metalness);
	vec3 diffuseIBL = m_Params.Albedo * irradiance;
	
	int envRadianceTexLevels = textureQueryLevels(u_EnvRadianceTex);
	float NoV = clamp(m_Params.NdotV, 0.0, 1.0);
	vec3 R = 2.0 * dot(m_Params.View, m_Params.Normal) * m_Params.Normal - m_Params.View;
	vec3 specularIrradiance = textureLod(u_EnvRadianceTex, RotateVectorAboutY(u_EnvMapRotation, Lr), (m_Params.Roughness) * envRadianceTexLevels).rgb;
	
	// Sample BRDF Lut, 1.0 - roughness for y-coord because texture was generated (in Sparky) for gloss model
	vec2 specularBRDF = texture(u_BRDFLUTTexture, vec2(m_Params.NdotV, 1.0 - m_Params.Roughness)).rg;
	vec3 specularIBL = specularIrradiance * (F0 * specularBRDF.x + specularBRDF.y);
	
	return (kd* diffuseIBL + specularIBL) / 2;
}

/////////////////////////////////////////////
// PCSS
/////////////////////////////////////////////

uint CascadeIndex = 0;
float ShadowFade = 1.0;

float GetShadowBias()
{
	const float MINIMUM_SHADOW_BIAS = 0.002;
	float bias = max(MINIMUM_SHADOW_BIAS * (1.0 - dot(m_Params.Normal, u_Light.environmentLight[0].Direction)), MINIMUM_SHADOW_BIAS);
	return bias;
}

float HardShadows_DirectionalLight(sampler2D shadowMap, vec3 shadowCoords)
{
	float bias = GetShadowBias();
	float z = texture(shadowMap, shadowCoords.xy).x;
	return 1.0 - step(z + bias, shadowCoords.z) * ShadowFade;
}
// Penumbra

// this search area estimation comes from the following article: 
// http://developer.download.nvidia.com/whitepapers/2008/PCSS_Integration.pdf
float SearchWidth(float uvLightSize, float receiverDistance)
{
	const float NEAR = 0.1;
	return uvLightSize * (receiverDistance - NEAR) / vs_Input.CameraPosition.z;
}

float u_light_zNear = 0.0; // 0.01 gives artifacts? maybe because of ortho proj?
float u_light_zFar = 10000.0;
vec2 u_lightRadiusUV = vec2(0.05);
vec2 searchRegionRadiusUV(float zWorld)
{
    return u_lightRadiusUV * (zWorld - u_light_zNear) / zWorld;
}

const vec2 PoissonDistribution[64] = vec2[](
	vec2(-0.884081, 0.124488),
	vec2(-0.714377, 0.027940),
	vec2(-0.747945, 0.227922),
	vec2(-0.939609, 0.243634),
	vec2(-0.985465, 0.045534),
	vec2(-0.861367, -0.136222),
	vec2(-0.881934, 0.396908),
	vec2(-0.466938, 0.014526),
	vec2(-0.558207, 0.212662),
	vec2(-0.578447, -0.095822),
	vec2(-0.740266, -0.095631),
	vec2(-0.751681, 0.472604),
	vec2(-0.553147, -0.243177),
	vec2(-0.674762, -0.330730),
	vec2(-0.402765, -0.122087),
	vec2(-0.319776, -0.312166),
	vec2(-0.413923, -0.439757),
	vec2(-0.979153, -0.201245),
	vec2(-0.865579, -0.288695),
	vec2(-0.243704, -0.186378),
	vec2(-0.294920, -0.055748),
	vec2(-0.604452, -0.544251),
	vec2(-0.418056, -0.587679),
	vec2(-0.549156, -0.415877),
	vec2(-0.238080, -0.611761),
	vec2(-0.267004, -0.459702),
	vec2(-0.100006, -0.229116),
	vec2(-0.101928, -0.380382),
	vec2(-0.681467, -0.700773),
	vec2(-0.763488, -0.543386),
	vec2(-0.549030, -0.750749),
	vec2(-0.809045, -0.408738),
	vec2(-0.388134, -0.773448),
	vec2(-0.429392, -0.894892),
	vec2(-0.131597, 0.065058),
	vec2(-0.275002, 0.102922),
	vec2(-0.106117, -0.068327),
	vec2(-0.294586, -0.891515),
	vec2(-0.629418, 0.379387),
	vec2(-0.407257, 0.339748),
	vec2(0.071650, -0.384284),
	vec2(0.022018, -0.263793),
	vec2(0.003879, -0.136073),
	vec2(-0.137533, -0.767844),
	vec2(-0.050874, -0.906068),
	vec2(0.114133, -0.070053),
	vec2(0.163314, -0.217231),
	vec2(-0.100262, -0.587992),
	vec2(-0.004942, 0.125368),
	vec2(0.035302, -0.619310),
	vec2(0.195646, -0.459022),
	vec2(0.303969, -0.346362),
	vec2(-0.678118, 0.685099),
	vec2(-0.628418, 0.507978),
	vec2(-0.508473, 0.458753),
	vec2(0.032134, -0.782030),
	vec2(0.122595, 0.280353),
	vec2(-0.043643, 0.312119),
	vec2(0.132993, 0.085170),
	vec2(-0.192106, 0.285848),
	vec2(0.183621, -0.713242),
	vec2(0.265220, -0.596716),
	vec2(-0.009628, -0.483058),
	vec2(-0.018516, 0.435703)
);

const vec2 poissonDisk[16] = vec2[](
 vec2( -0.94201624, -0.39906216 ),
 vec2( 0.94558609, -0.76890725 ),
 vec2( -0.094184101, -0.92938870 ),
 vec2( 0.34495938, 0.29387760 ),
 vec2( -0.91588581, 0.45771432 ),
 vec2( -0.81544232, -0.87912464 ),
 vec2( -0.38277543, 0.27676845 ),
 vec2( 0.97484398, 0.75648379 ),
 vec2( 0.44323325, -0.97511554 ),
 vec2( 0.53742981, -0.47373420 ),
 vec2( -0.26496911, -0.41893023 ),
 vec2( 0.79197514, 0.19090188 ),
 vec2( -0.24188840, 0.99706507 ),
 vec2( -0.81409955, 0.91437590 ),
 vec2( 0.19984126, 0.78641367 ),
 vec2( 0.14383161, -0.14100790 )
); 

vec2 SamplePoisson(int index)
{
   return PoissonDistribution[index % 64];
}

float FindBlockerDistance_DirectionalLight(sampler2D shadowMap, vec3 shadowCoords, float uvLightSize)
{
	float bias = GetShadowBias();

	int numBlockerSearchSamples = 64;
	int blockers = 0;
	float avgBlockerDistance = 0;

	float zEye = -(u_LightView * vec4(vs_Input.WorldPosition, 1.0)).z;
	vec2 searchWidth = searchRegionRadiusUV(zEye);
	for (int i = 0; i < numBlockerSearchSamples; i++)
	{
		float z = textureLod(shadowMap, (shadowCoords.xy * 0.5 + 0.5) + SamplePoisson(i) * searchWidth, 0).r;
		if (z < (shadowCoords.z - bias))
		{
			blockers++;
			avgBlockerDistance += z;
		}
	}

	if (blockers > 0)
		return avgBlockerDistance / float(blockers);

	return -1;
}

float PCF_DirectionalLight(sampler2D shadowMap, vec3 shadowCoords, float uvRadius)
{
	float bias = GetShadowBias();
	int numPCFSamples = 64;
	
	float sum = 0;
	for (int i = 0; i < numPCFSamples; i++)
	{
		vec2 offset = SamplePoisson(i) * uvRadius;
		float z = textureLod(shadowMap, (shadowCoords.xy * 0.5 + 0.5) + offset, 0).r;
		sum += step(shadowCoords.z - bias, z);
	}
	return sum / numPCFSamples;
}

float NV_PCF_DirectionalLight(sampler2D shadowMap, vec3 shadowCoords, float uvRadius)
{
	float bias = GetShadowBias();

	float sum = 0;
	for (int i = 0; i < 16; i++)
	{
		vec2 offset = poissonDisk[i] * uvRadius;
		float z = textureLod(shadowMap, (shadowCoords.xy * 0.5 + 0.5) + offset, 0).r;
		sum += step(shadowCoords.z - bias, z);
	}
	return sum / 16.0f;
}

float PCSS_DirectionalLight(sampler2D shadowMap, vec3 shadowCoords, float uvLightSize)
{
	float blockerDistance = FindBlockerDistance_DirectionalLight(shadowMap, shadowCoords, uvLightSize);
	if (blockerDistance == -1) // No occlusion
		return 1.0f;

	float penumbraWidth = (shadowCoords.z - blockerDistance) / blockerDistance;

	float NEAR = 0.01; // Should this value be tweakable?
	float uvRadius = penumbraWidth * uvLightSize * NEAR / shadowCoords.z; // Do we need to divide by shadowCoords.z?
	uvRadius = min(uvRadius, 0.002f);
	return PCF_DirectionalLight(shadowMap, shadowCoords, uvRadius) * ShadowFade;
}

/////////////////////////////////////////////

void main()
{
	// Standard PBR inputs
	 m_Params.Albedo = u_UseAlbedoTexture ?  texture(u_AlbedoTexture, vs_Input.TexCoord).rgb
                                            : u_AlbedoColor;

    m_Params.Metalness = u_UseMetalnessTexture ? texture(u_MetalnessTexture, vs_Input.TexCoord).r
                                                : u_Metalness;

    m_Params.Roughness = u_UseRoughnessTexture ? texture(u_RoughnessTexture, vs_Input.TexCoord).r
                                                : u_Roughness;

	// Normals (either from vertex or map)
	m_Params.Normal = normalize(vs_Input.Normal);

	m_Params.View = normalize(vs_Input.CameraPosition - vs_Input.WorldPosition);
	m_Params.NdotV = max(dot(m_Params.Normal, m_Params.View), 0.0);
		
	// Specular reflection vector
	vec3 Lr = 2.0 * m_Params.NdotV * m_Params.Normal - m_Params.View;
	
	// Fresnel reflectance, metals use albedo
	vec3 F0 = mix(Fdielectric, m_Params.Albedo, m_Params.Metalness);

	//float shadowAmount = ShadowMap(vs_Input.ShadowMapCoords);

    float shadowAmount = 1.0;
    float cascadeTransitionFade = 1.0; // u_CascadeTransitionFade;

    const uint SHADOW_MAP_CASCADE_COUNT = 4;
	for(uint i = 0; i < SHADOW_MAP_CASCADE_COUNT - 1; i++)
	{
		if(vs_Input.ViewPosition.z < u_CascadeSplits[i])
			CascadeIndex = i + 1;
	}
    float shadowDistance = 200.0;
	float transitionDistance = 25.0;
	float distance = length(vs_Input.ViewPosition);
	ShadowFade = distance - (shadowDistance - transitionDistance);
	ShadowFade /= transitionDistance;
	ShadowFade = clamp(1.0 - ShadowFade, 0.0, 1.0);

    vec3 shadowMapCoords = (vs_Input.ShadowMapCoords[CascadeIndex].xyz / vs_Input.ShadowMapCoords[CascadeIndex].w);

    shadowAmount = HardShadows_DirectionalLight(u_ShadowMapTexture[CascadeIndex], shadowMapCoords);

	vec3 lightContribution = u_Light.environmentLight[0].Intensity > 0.0 ?  (Lighting(F0)  * shadowAmount) : vec3(0.0);
	vec3 iblContribution = IBL(F0, Lr) * u_IBLContribution;


	vec3 pointLights = vec3(0.0);
	for(uint i = 0; i < u_Light.pointLight.size; i++)
	{
		pointLights += CalculatePointLight(u_Light.pointLight.pointLights[i], vs_Input.WorldPosition);
	}

	vec3 finalLightning = lightContribution + iblContribution + pointLights;
	o_Color = vec4(finalLightning, 1.0);
   
}
