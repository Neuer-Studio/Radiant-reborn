#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

#include <Radiant/Core/Application.hpp>

#include <Radiant/Rendering/Image.hpp>
#include <Radiant/Rendering/Rendering.hpp>
#include <Radiant/Rendering/Framebuffer.hpp>
#include <Radiant/Rendering/Pipeline.hpp>
#include <Radiant/Rendering/Material.hpp>
#include <Radiant/Rendering/Mesh.hpp>
#include <Radiant/Scene/Components.hpp>
#include <Radiant/Rendering/Platform/OpenGL/OpenGLImage.hpp>

#include <Radiant/Scene/Scene.hpp>

#include <Radiant/Scene/SceneRendering.hpp>

namespace Radiant
{
	static constexpr int kShadowMapSize = 4096;
	static constexpr int kBRDF_LUT_Size = 256;
	static constexpr int kLightEnvironmentSize = sizeof(LightEnvironment);

	struct GeometryData
	{
		Memory::Shared<Pipeline> pipeline;
		Memory::Shared<Material> material;
	};

	struct ShadowData
	{
		Memory::Shared<Pipeline> ShadowPassPipeline[4];
		Memory::Shared<Material> ShadowMapMaterial;
		float ShadowMapSize = 20.0f;
		float LightDistance = 0.1f;
		glm::mat4 LightMatrices[4];
		glm::mat4 LightViewMatrix;
		float CascadeSplitLambda = 0.91f;
		glm::vec4 CascadeSplits;
		float CascadeFarPlaneOffset = 15.0f, CascadeNearPlaneOffset = -15.0f;
		bool ShowCascades = false;
		bool SoftShadows = true;
		float LightSize = 0.5f;
		float MaxShadowDistance = 200.0f;
		float ShadowFade = 25.0f;
		float CascadeTransitionFade = 1.0f;
		bool CascadeFading = true;

		RenderingID ShadowMapSampler;
	};

	struct CompositeData // TODO: Move to RenderPass
	{
		Memory::Shared<Pipeline> pipeline;
		Memory::Shared<Material> material;
	};

	struct RenderPassList
	{
		GeometryData GeoData;
		CompositeData CompData;
		ShadowData Shadowdata;
	};

	struct DrawCommand
	{
		glm::mat4 Transform;
		Memory::Shared<Mesh> Mesh;
	};

	struct SceneInfo
	{
		struct RenderPassList RenderPassList;
		Memory::Shared<Shader> DefaultShader;
		std::vector<DrawCommand> MeshDrawList;
		Memory::Shared<Pipeline> GridPipeline;
		Memory::Shared<Material> GridMaterial;
		Memory::Shared<Pipeline> SkyboxPipeline;
		Memory::Shared<Material> SkyboxMaterial;
		struct LightEnvironment LightEnvironment;
		Memory::Shared<Texture2D> BRDF_LUT;

		struct
		{
			glm::mat4 ViewProjection;
			glm::mat4 View;
			glm::mat4 Projection;
			glm::mat4 InversedViewProjection;
			glm::vec3 CameraPos;
			float Exposure;
		} SceneCamera;

		bool ShowGrid = true;

		uint32_t ViewportWidth;
		uint32_t ViewportHeight;
		Memory::Shared<Scene> ActiveScene;
		Environment EnvironmentMap;
		EnvironmentAttributes Attributes;
	};

	static SceneInfo* s_SceneInfo = nullptr;

	SceneRendering::~SceneRendering()
	{
		delete s_SceneInfo;
	}

	void SceneRendering::BeginScene(Memory::Shared<Scene> scene, const Camera& camera)
	{
		RADIANT_VERIFY(s_SceneInfo, "Did you call Init() ?");
		RADIANT_VERIFY(!s_SceneInfo->ActiveScene, "There active scene! Can you call EndScene()?");
		RADIANT_VERIFY(scene);
		s_SceneInfo->ActiveScene = scene;

		s_SceneInfo->SceneCamera.ViewProjection = camera.GetViewProjection();
		s_SceneInfo->SceneCamera.View = camera.GetViewMatrix();
		s_SceneInfo->SceneCamera.Projection = camera.GetProjectionMatrix();
		s_SceneInfo->SceneCamera.CameraPos = camera.GetPosition();
		s_SceneInfo->SceneCamera.Exposure = 0.8f;// camera.GetExposure();
		s_SceneInfo->SceneCamera.InversedViewProjection = glm::inverse(camera.GetViewProjection());

		s_SceneInfo->LightEnvironment = s_SceneInfo->ActiveScene->GetLightEnvironment();

	}

	void SceneRendering::EndScene()
	{
		RADIANT_VERIFY(s_SceneInfo->ActiveScene, "No active scene! Can you call BeginScene()?");
		s_SceneInfo->ActiveScene = nullptr;
	}

	void SceneRendering::Init()
	{
		s_SceneInfo = new SceneInfo();

		s_SceneInfo->ViewportWidth = 1280;
		s_SceneInfo->ViewportHeight = 720;

		// Geometry pass

		{
			RenderPassSpecification renderPassSpec;
			renderPassSpec.TargetFramebuffer = Framebuffer::Create({ s_SceneInfo->ViewportWidth, s_SceneInfo->ViewportHeight, 8, { ImageFormat::RGBA16F, ImageFormat::DEPTH32F } });
			renderPassSpec.DebugName = "Geometry Render Pass";

			PipelineSpecification pipelineSpecification;
			pipelineSpecification.Layout = {
				{ ShaderDataType::Float3, "a_Position" },
				{ ShaderDataType::Float3, "a_Normals" },
				{ ShaderDataType::Float2, "a_TexCoord" },
				{ ShaderDataType::Float3, "a_Tangent" },
				{ ShaderDataType::Float3, "a_Bitangent" }
			};

			pipelineSpecification.DebugName = "PBR-Static";
			pipelineSpecification.RenderPass = RenderPass::Create(renderPassSpec);
			pipelineSpecification.Shader = Rendering::GetShaderLibrary()->Get("StaticPBR_Radiant.glsl");

			s_SceneInfo->RenderPassList.GeoData.pipeline = Pipeline::Create(pipelineSpecification);
			s_SceneInfo->RenderPassList.GeoData.material = Material::Create(pipelineSpecification.Shader);
		}

		// Composite pass

		{
			RenderPassSpecification renderPassSpec;
			renderPassSpec.TargetFramebuffer = Framebuffer::Create({ s_SceneInfo->ViewportWidth, s_SceneInfo->ViewportHeight, 1, { ImageFormat::RGBA16F } });
			renderPassSpec.DebugName = "Composite Render Pass";

			PipelineSpecification pipelineSpecification;
			pipelineSpecification.Layout = {
				{ ShaderDataType::Float3, "a_Position" },
				{ ShaderDataType::Float2, "a_TexCoord" }
			};

			pipelineSpecification.DebugName = "Scene Composite";
			pipelineSpecification.RenderPass = RenderPass::Create(renderPassSpec);
			pipelineSpecification.Shader = Rendering::GetShaderLibrary()->Get("SceneCompositeMSAA.glsl"); //TODO
			s_SceneInfo->RenderPassList.CompData.pipeline = Pipeline::Create(pipelineSpecification);

			s_SceneInfo->RenderPassList.CompData.material = Material::Create(pipelineSpecification.Shader);
		}

		// Grid

		{
			const auto& gridShader = Rendering::GetShaderLibrary()->Get("Grid.glsl");
			s_SceneInfo->GridMaterial = Material::Create(gridShader);
			s_SceneInfo->GridMaterial->SetFlag(MaterialFlag::TwoSided, true);
			s_SceneInfo->GridMaterial->SetFlag(MaterialFlag::DepthTest, true);

			PipelineSpecification pipelineSpec;
			pipelineSpec.DebugName = "Grid";
			pipelineSpec.Shader = gridShader;
			pipelineSpec.Layout = {
				{ ShaderDataType::Float3, "a_Position" },
				{ ShaderDataType::Float2, "a_TexCoord" }
			};
			pipelineSpec.RenderPass = s_SceneInfo->RenderPassList.GeoData.pipeline->GetSpecification().RenderPass;
			s_SceneInfo->GridPipeline = Pipeline::Create(pipelineSpec);
		}

		// Skybox

		{
			const auto& skyboxShader = Rendering::GetShaderLibrary()->Get("Skybox.glsl");

			PipelineSpecification pipelineSpec;
			pipelineSpec.DebugName = "Skybox";
			pipelineSpec.Shader = skyboxShader;
			pipelineSpec.Layout = {
				{ ShaderDataType::Float3, "a_Position" },
				{ ShaderDataType::Float2, "a_TexCoord" }
			};
			s_SceneInfo->SkyboxPipeline = Pipeline::Create(pipelineSpec);
			s_SceneInfo->SkyboxMaterial = Material::Create(skyboxShader);
		}

		// Shadow map

		{
			PipelineSpecification ps;
			ps.DebugName = "Shadow map";
			ps.Layout = {
				{ ShaderDataType::Float3, "a_Position" },
				{ ShaderDataType::Float3, "a_Normals" },
				{ ShaderDataType::Float2, "a_TexCoord" },
				{ ShaderDataType::Float3, "a_Tangent" },
				{ ShaderDataType::Float3, "a_Bitangent" }
			};
			ps.Shader = Rendering::GetShaderLibrary()->Get("ShadowMap.glsl");
			s_SceneInfo->RenderPassList.Shadowdata.ShadowMapMaterial = Material::Create(ps.Shader);

			// 4 cascades
			for (int i = 0; i < 4; i++)
			{
				RenderPassSpecification shadowMapRenderPassSpec;
				shadowMapRenderPassSpec.TargetFramebuffer = Framebuffer::Create({ kShadowMapSize, kShadowMapSize, 1, { ImageFormat::DEPTH32F } });
				shadowMapRenderPassSpec.DebugName = "Geometry Render Pass";
				ps.RenderPass = RenderPass::Create(shadowMapRenderPassSpec);

				s_SceneInfo->RenderPassList.Shadowdata.ShadowPassPipeline[i] = Pipeline::Create(ps);
			}

			Rendering::SubmitCommand([]()
				{
					glGenSamplers(1, &s_SceneInfo->RenderPassList.Shadowdata.ShadowMapSampler);

					// Setup the shadowmap depth sampler
					glSamplerParameteri(s_SceneInfo->RenderPassList.Shadowdata.ShadowMapSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glSamplerParameteri(s_SceneInfo->RenderPassList.Shadowdata.ShadowMapSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glSamplerParameteri(s_SceneInfo->RenderPassList.Shadowdata.ShadowMapSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
					glSamplerParameteri(s_SceneInfo->RenderPassList.Shadowdata.ShadowMapSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				});

		}

		s_SceneInfo->BRDF_LUT = Texture2D::Create("Resources/Textures/BRDF_LUT.tga");
		s_SceneInfo->MeshDrawList.reserve(100); // TODO: Add a capcaity from YAML(scene)

	}

	void SceneRendering::SetSceneVeiwPortSize(const glm::vec2& size)
	{
		RADIANT_VERIFY(s_SceneInfo, "Did you call Init() ?");
		RADIANT_VERIFY(s_SceneInfo->ActiveScene, "Did you call BeginScene() ?");

		if (s_SceneInfo->ViewportWidth != size.x || s_SceneInfo->ViewportHeight != size.y)
		{
			s_SceneInfo->ViewportWidth = size.x;
			s_SceneInfo->ViewportHeight = size.y;

			s_SceneInfo->RenderPassList.GeoData.pipeline->GetSpecification().RenderPass->GetSpecification().TargetFramebuffer->Resize(size.x, size.y);
			s_SceneInfo->RenderPassList.CompData.pipeline->GetSpecification().RenderPass->GetSpecification().TargetFramebuffer->Resize(size.x, size.y);
		}
	}

	void SceneRendering::SetEnvironment(const Environment& env)
	{
		s_SceneInfo->EnvironmentMap = env;
		TextureDescriptor descriptor;

		descriptor.Name = "u_EnvTexture";
		s_SceneInfo->SkyboxMaterial->SetImage2D(descriptor, env.Radiance);
	}

	void SceneRendering::SetEnvironmentAttributes(const EnvironmentAttributes& attributes)
	{
		RADIANT_VERIFY(s_SceneInfo, "Did you call Init() ?");
		s_SceneInfo->Attributes = attributes;
	}

	void SceneRendering::SetEnvMapRotation(float rotation)
	{
		RADIANT_VERIFY(s_SceneInfo, "Did you call Init() ?");

	}

	void SceneRendering::SetIBLContribution(float value)
	{
		RADIANT_VERIFY(s_SceneInfo, "Did you call Init() ?");

	}

	void SceneRendering::OnImGuiRender()
	{
		RADIANT_VERIFY(s_SceneInfo, "Did you call Init() ?");

	}

	void SceneRendering::SubmitMesh(const Memory::Shared<Mesh>& mesh, const glm::mat4& transform)
	{
		RADIANT_VERIFY(s_SceneInfo, "Did you call Init() ?");
		s_SceneInfo->MeshDrawList.push_back({ transform, mesh });
	}

	Radiant::Memory::Shared<Radiant::Image2D> SceneRendering::GetFinalPassImage()
	{
		RADIANT_VERIFY(s_SceneInfo, "Did you call Init() ?");
		return s_SceneInfo->RenderPassList.CompData.pipeline->GetSpecification().RenderPass->GetSpecification().TargetFramebuffer->GetColorAttachmentImage(0);
	}

	Radiant::Memory::Shared<Radiant::Image2D> SceneRendering::GetShadowMapPassImage()
	{
		RADIANT_VERIFY(s_SceneInfo, "Did you call Init() ?");
		return nullptr;
	}

	Radiant::Environment SceneRendering::CreateEnvironmentMap(const std::filesystem::path& filepath)
	{
		RADIANT_VERIFY(s_SceneInfo, "Did you call Init() ?");
		return Rendering::CreateEnvironmentMap(filepath);
	}

	struct FrustumBounds
	{
		float r, l, b, t, f, n;
	};

	struct CascadeData
	{
		glm::mat4 ViewProj;
		glm::mat4 View;
		float SplitDepth;
	};

	static void CalculateCascades(CascadeData* cascades, const glm::vec3& lightDirection)
	{
		FrustumBounds frustumBounds[3];

		auto viewProjection = s_SceneInfo->SceneCamera.ViewProjection;

		const int SHADOW_MAP_CASCADE_COUNT = 4;
		float cascadeSplits[SHADOW_MAP_CASCADE_COUNT];

		// TODO: less hard-coding!
		float nearClip = 0.1f;
		float farClip = 1000.0f;
		float clipRange = farClip - nearClip;

		float minZ = nearClip;
		float maxZ = nearClip + clipRange;

		float range = maxZ - minZ;
		float ratio = maxZ / minZ;

		// Calculate split depths based on view camera frustum
		// Based on method presented in https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html
		for (uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++)
		{
			float p = (i + 1) / static_cast<float>(SHADOW_MAP_CASCADE_COUNT);
			float log = minZ * std::pow(ratio, p);
			float uniform = minZ + range * p;
			float d = s_SceneInfo->RenderPassList.Shadowdata.CascadeSplitLambda * (log - uniform) + uniform;
			cascadeSplits[i] = (d - nearClip) / clipRange;
		}

		cascadeSplits[3] = 0.3f;

		// Manually set cascades here
		// cascadeSplits[0] = 0.05f;
		// cascadeSplits[1] = 0.15f;
		// cascadeSplits[2] = 0.3f;
		// cascadeSplits[3] = 1.0f;

		// Calculate orthographic projection matrix for each cascade
		float lastSplitDist = 0.0;
		for (uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++)
		{
			float splitDist = cascadeSplits[i];

			glm::vec3 frustumCorners[8] =
			{
				glm::vec3(-1.0f,  1.0f, -1.0f),
				glm::vec3(1.0f,  1.0f, -1.0f),
				glm::vec3(1.0f, -1.0f, -1.0f),
				glm::vec3(-1.0f, -1.0f, -1.0f),
				glm::vec3(-1.0f,  1.0f,  1.0f),
				glm::vec3(1.0f,  1.0f,  1.0f),
				glm::vec3(1.0f, -1.0f,  1.0f),
				glm::vec3(-1.0f, -1.0f,  1.0f),
			};

			// Project frustum corners into world space
			glm::mat4 invCam = glm::inverse(viewProjection);
			for (uint32_t i = 0; i < 8; i++)
			{
				glm::vec4 invCorner = invCam * glm::vec4(frustumCorners[i], 1.0f);
				frustumCorners[i] = invCorner / invCorner.w;
			}

			for (uint32_t i = 0; i < 4; i++)
			{
				glm::vec3 dist = frustumCorners[i + 4] - frustumCorners[i];
				frustumCorners[i + 4] = frustumCorners[i] + (dist * splitDist);
				frustumCorners[i] = frustumCorners[i] + (dist * lastSplitDist);
			}

			// Get frustum center
			glm::vec3 frustumCenter = glm::vec3(0.0f);
			for (uint32_t i = 0; i < 8; i++)
				frustumCenter += frustumCorners[i];

			frustumCenter /= 8.0f;

			//frustumCenter *= 0.01f;

			float radius = 0.0f;
			for (uint32_t i = 0; i < 8; i++)
			{
				float distance = glm::length(frustumCorners[i] - frustumCenter);
				radius = glm::max(radius, distance);
			}
			radius = std::ceil(radius * 16.0f) / 16.0f;

			glm::vec3 maxExtents = glm::vec3(radius);
			glm::vec3 minExtents = -maxExtents;

			glm::vec3 lightDir = -lightDirection;
			glm::mat4 lightViewMatrix = glm::lookAt(frustumCenter - lightDir * -minExtents.z, frustumCenter, glm::vec3(0.0f, 0.0f, 1.0f));
			glm::mat4 lightOrthoMatrix = glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, 0.0f + s_SceneInfo->RenderPassList.Shadowdata.CascadeNearPlaneOffset, maxExtents.z - minExtents.z + s_SceneInfo->RenderPassList.Shadowdata.CascadeFarPlaneOffset);

			// Store split distance and matrix in cascade
			cascades[i].SplitDepth = (nearClip + splitDist * clipRange) * -1.0f;
			cascades[i].ViewProj = lightOrthoMatrix * lightViewMatrix;
			cascades[i].View = lightViewMatrix;

			lastSplitDist = cascadeSplits[i];
		}
	}

	void GeometryPass()
	{
		Rendering::BeginRenderPass(s_SceneInfo->RenderPassList.GeoData.pipeline->GetSpecification().RenderPass);

		s_SceneInfo->SkyboxPipeline->GetSpecification().Shader->Use();
		Rendering::SubmitFullscreenQuad(s_SceneInfo->SkyboxPipeline, nullptr);

		for (const auto& mesh : s_SceneInfo->MeshDrawList)
		{
			const auto& diffuse = mesh.Mesh->GetMaterialDiffuseData();
			const auto& normal = mesh.Mesh->GetMaterialNormalData();
			const auto& roughness = mesh.Mesh->GetMaterialRoughnessData();
			const auto& metalness = mesh.Mesh->GetMaterialMetalnessData();

			/*if (diffuse.Enabled)
			{
				s_SceneInfo->RenderPassList.GeoData.material->SetImage2D("u_AlbedoTexture", diffuse.Texture);
				if (normal.Enabled)
					s_SceneInfo->RenderPassList.GeoData.material->SetImage2D("u_NormalTexture", normal.Texture);
			}

			s_SceneInfo->RenderPassList.GeoData.material->SetImage2D("u_RoughnessTexture", roughness.Texture);
			s_SceneInfo->RenderPassList.GeoData.material->SetImage2D("u_MetalnessTexture", metalness.Texture);*/

			s_SceneInfo->RenderPassList.GeoData.material->SetFloat("u_Roughness", roughness.Roughness);
			s_SceneInfo->RenderPassList.GeoData.material->SetFloat("u_Metalness", metalness.Metalness);
			s_SceneInfo->RenderPassList.GeoData.material->SetVec3("u_AlbedoColor", diffuse.AlbedoColor);

			//UBO
			Material::SetUBO(2, "u_EnvironmentLight", &s_SceneInfo->LightEnvironment.DirectionalLights, kLightEnvironmentSize);
			Material::SetUBO(2, "u_EnvMapRotation", 0.0f);//todo
			Material::SetUBO(2, "u_IBLContribution", 1.0f);//todo

			//Update toggles
			s_SceneInfo->RenderPassList.GeoData.material->SetBool("u_UseNormalTexture", normal.Enabled);
			s_SceneInfo->RenderPassList.GeoData.material->SetBool("u_UseAlbedoTexture", diffuse.Enabled);
			s_SceneInfo->RenderPassList.GeoData.material->SetBool("u_UseMetalnessTexture", metalness.Enabled);
			s_SceneInfo->RenderPassList.GeoData.material->SetBool("u_UseRoughnessTexture", roughness.Enabled);

			// Env. map
			TextureDescriptor descriptor;

			descriptor.Name = "u_EnvRadianceTex";
			s_SceneInfo->RenderPassList.GeoData.material->SetImage2D(descriptor, s_SceneInfo->EnvironmentMap.Radiance); // TODO: create ubo, contatins the textures

			descriptor.Name = "u_EnvIrradianceTex";
			s_SceneInfo->RenderPassList.GeoData.material->SetImage2D(descriptor, s_SceneInfo->EnvironmentMap.Irradiance);

			descriptor.Name = "u_BRDFLUTTexture";
			s_SceneInfo->RenderPassList.GeoData.material->SetImage2D(descriptor, s_SceneInfo->BRDF_LUT->GetImage2D());

			//Shadow

			TextureDescriptor shadowDescriptor;
			shadowDescriptor.Name = "u_ShadowMapTexture";
			shadowDescriptor.Sampler = s_SceneInfo->RenderPassList.Shadowdata.ShadowMapSampler;

			for (int i = 0; i < 4; i++)
			{
				shadowDescriptor.ArrayIndex = i;

				s_SceneInfo->RenderPassList.GeoData.material->SetMat4("u_LightMatrixCascade", s_SceneInfo->RenderPassList.Shadowdata.LightMatrices[i], i);
				s_SceneInfo->RenderPassList.GeoData.material->SetImage2D(shadowDescriptor, s_SceneInfo->RenderPassList.Shadowdata.ShadowPassPipeline[i]->
					GetSpecification().RenderPass->GetSpecification().TargetFramebuffer->GetDepthAttachmentImage());
			}

			s_SceneInfo->RenderPassList.GeoData.material->SetVec4("u_CascadeSplits", s_SceneInfo->RenderPassList.Shadowdata.CascadeSplits);
			s_SceneInfo->RenderPassList.GeoData.material->SetMat4("u_LightView", s_SceneInfo->RenderPassList.Shadowdata.LightViewMatrix);

			Rendering::SubmitMeshWithMaterial({ mesh.Transform, mesh.Mesh, s_SceneInfo->RenderPassList.GeoData.material }, s_SceneInfo->RenderPassList.GeoData.pipeline);
		}

		if (s_SceneInfo->ShowGrid)
		{
			Rendering::SubmitFullscreenQuad(s_SceneInfo->GridPipeline, s_SceneInfo->GridMaterial);
		}

		Rendering::EndRenderPass();
	}

	void ShadowMapPass()
	{
		auto& directionalLights = s_SceneInfo->LightEnvironment.DirectionalLights;
		if (directionalLights.Multiplier == 0.0f)
		{
			for (int i = 0; i < 4; i++)
			{
				// Clear shadow maps
				Rendering::BeginRenderPass(s_SceneInfo->RenderPassList.Shadowdata.ShadowPassPipeline[i]->GetSpecification().RenderPass);
				Rendering::EndRenderPass();
			}
			return;
		}
		Rendering::SubmitCommand([]()
			{
				glEnable(GL_CULL_FACE);
				glCullFace(GL_BACK);
			});

		CascadeData cascades[4];
		CalculateCascades(cascades, directionalLights.Direction);
		s_SceneInfo->RenderPassList.Shadowdata.LightViewMatrix = cascades[0].View;

		for (int i = 0; i < 4; i++)
		{
			Rendering::BeginRenderPass(s_SceneInfo->RenderPassList.Shadowdata.ShadowPassPipeline[i]->GetSpecification().RenderPass);

			s_SceneInfo->RenderPassList.Shadowdata.CascadeSplits[i] = cascades[i].SplitDepth;
			glm::mat4 shadowMapVP = cascades[i].ViewProj;
			s_SceneInfo->RenderPassList.Shadowdata.ShadowMapMaterial->SetMat4("u_ViewProjection", shadowMapVP);

			static glm::mat4 scaleBiasMatrix = glm::scale(glm::mat4(1.0f), { 0.5f, 0.5f, 0.5f }) * glm::translate(glm::mat4(1.0f), { 1, 1, 1 });
			s_SceneInfo->RenderPassList.Shadowdata.LightMatrices[i] = scaleBiasMatrix * cascades[i].ViewProj;

			for (const auto& mesh : s_SceneInfo->MeshDrawList)
			{
				Rendering::SubmitMeshWithMaterial({ mesh.Transform, mesh.Mesh, s_SceneInfo->RenderPassList.Shadowdata.ShadowMapMaterial }, s_SceneInfo->RenderPassList.Shadowdata.ShadowPassPipeline[i]);
			}

			Rendering::EndRenderPass();
		}

		Rendering::SubmitCommand([]()
			{
				glDisable(GL_CULL_FACE);
			});
	}

	void CompositePass()
	{
		Rendering::BeginRenderPass(s_SceneInfo->RenderPassList.CompData.pipeline->GetSpecification().RenderPass);
		s_SceneInfo->RenderPassList.CompData.material->SetFloat("u_Exposure", s_SceneInfo->SceneCamera.Exposure); //TODO: move to the UBO
		s_SceneInfo->RenderPassList.CompData.material->SetUint("u_SamplesCount", s_SceneInfo->ActiveScene->GetSceneSamplesCount());

		Material::SetUBO(10, "u_TextureLod", s_SceneInfo->Attributes.EnvironmentMapLod);
		Material::SetUBO(10, "u_SkyIntensity", s_SceneInfo->Attributes.Intensity);

		TextureDescriptor descriptor;
		descriptor.Name = "u_Texture";
		s_SceneInfo->RenderPassList.CompData.material->SetImage2D(descriptor, s_SceneInfo->RenderPassList.GeoData.pipeline->GetSpecification().RenderPass->GetSpecification().TargetFramebuffer->GetColorAttachmentImage());
		s_SceneInfo->RenderPassList.CompData.pipeline->GetSpecification().Shader->Use();
		Rendering::SubmitFullscreenQuad(s_SceneInfo->RenderPassList.CompData.pipeline, nullptr);
		Rendering::EndRenderPass();
	}

	void FlushDrawList()
	{
		ShadowMapPass();
		GeometryPass();
		CompositePass();

		s_SceneInfo->MeshDrawList.clear(); // TODO: Optimize
	}

	void SceneRendering::OnUpdate(Timestep ts)
	{
		RADIANT_VERIFY(s_SceneInfo, "Did you call Init() ?");

		static const glm::mat4 transform = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(16.0f));

		FlushDrawList();

		Material::SetUBO(0, "u_ViewProjectionMatrix", s_SceneInfo->SceneCamera.ViewProjection);
		Material::SetUBO(0, "u_InversedViewProjectionMatrix", s_SceneInfo->SceneCamera.InversedViewProjection);
		Material::SetUBO(0, "u_ViewMatrix", s_SceneInfo->SceneCamera.View);
		Material::SetUBO(2, "u_CameraPosition", s_SceneInfo->SceneCamera.CameraPos);

		s_SceneInfo->GridMaterial->SetMat4("u_Transform", transform); //TODO: UBO

	}
}