#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

#include <Radiant/Core/Application.hpp>

#include <Radiant/Rendering/Platform/OpenGL/Scene/OpenGLSceneRendering.hpp>
#include <Radiant/Rendering/Image.hpp>
#include <Radiant/Rendering/Rendering.hpp>
#include <Radiant/Rendering/Framebuffer.hpp>
#include <Radiant/Rendering/Pipeline.hpp>
#include <Radiant/Rendering/Material.hpp>
#include <Radiant/Rendering/Mesh.hpp>
#include <Radiant/Scene/Component.hpp>
#include <Radiant/Rendering/Platform/OpenGL/OpenGLImage.hpp>

namespace Radiant
{
	static constexpr int kEnvMapSize = 2048;
	static constexpr int kShadowMapSize = 4096;
	static constexpr int kIrradianceMapSize = 32;
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
		} SceneCamera;

		bool Updated = false; // NOTE: Using for detect camera data update
		bool ShowGrid = true;
	};

	static SceneInfo* s_SceneInfo = nullptr;

	void OpenGLSceneRendering::Init()
	{
		m_ViewportWidth = Application::GetInstance().GetWindow()->GetWidth();
		m_ViewportHeight = Application::GetInstance().GetWindow()->GetHeight();

		s_SceneInfo = new SceneInfo();

		// Geometry pass

		{
			RenderPassSpecification renderPassSpec;
			renderPassSpec.TargetFramebuffer = Framebuffer::Create({ m_ViewportWidth, m_ViewportHeight, 8, { ImageFormat::RGBA16F, ImageFormat::DEPTH32F } });
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
			renderPassSpec.TargetFramebuffer = Framebuffer::Create({ m_ViewportWidth, m_ViewportHeight, 1, { ImageFormat::RGBA16F } });
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
			auto gridShader = Rendering::GetShaderLibrary()->Get("Grid.glsl");
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
			auto skyboxShader = Rendering::GetShaderLibrary()->Get("Skybox.glsl");

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

			Rendering::SubmitCommand([ ]()
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

		{
			Material::SetUBO(10, "u_TextureLod", GetTexureLod());
			Material::SetUBO(10, "u_SkyIntensity", 0.8f);
		}
	}

	Memory::Shared<Radiant::Image2D> OpenGLSceneRendering::GetFinalPassImage() const
	{
		return s_SceneInfo->RenderPassList.CompData.pipeline->GetSpecification().RenderPass->GetSpecification().TargetFramebuffer->GetColorAttachmentImage(0);
	}

	Radiant::Memory::Shared<Radiant::Image2D> OpenGLSceneRendering::GetShadowMapPassImage() const
	{
		return nullptr;
	}

	void OpenGLSceneRendering::OnImGuiRender()
	{

	}

	void OpenGLSceneRendering::SetSceneVeiwPortSize(const glm::vec2& size)
	{
		if (m_ViewportWidth != size.x || m_ViewportHeight != size.y)
		{
			m_ViewportWidth = size.x;
			m_ViewportHeight = size.y;

			s_SceneInfo->RenderPassList.GeoData.pipeline->GetSpecification().RenderPass->GetSpecification().TargetFramebuffer->Resize(size.x, size.y);
			s_SceneInfo->RenderPassList.CompData.pipeline->GetSpecification().RenderPass->GetSpecification().TargetFramebuffer->Resize(size.x, size.y);

			m_Scene->SetViewportSize(size.x, size.y);
		}
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

	void OpenGLSceneRendering::BeginScene(const Camera& camera)
	{
		s_SceneInfo->SceneCamera.ViewProjection = camera.GetViewProjection();
		s_SceneInfo->SceneCamera.View = camera.GetViewMatrix();
		s_SceneInfo->SceneCamera.Projection = camera.GetProjectionMatrix();
		s_SceneInfo->SceneCamera.CameraPos = camera.GetPosition();
		s_SceneInfo->SceneCamera.InversedViewProjection = glm::inverse(camera.GetViewProjection());

		s_SceneInfo->LightEnvironment = m_Scene->GetLightEnvironment();

		s_SceneInfo->Updated = true;
	}

	OpenGLSceneRendering::~OpenGLSceneRendering()
	{
		delete s_SceneInfo;
	}

	OpenGLSceneRendering::OpenGLSceneRendering(const Memory::Shared<Scene>& scene)
		: m_Scene(scene)
	{
		Init();
	}

	void OpenGLSceneRendering::OnUpdate(Timestep ts)
	{
		RADIANT_VERIFY(s_SceneInfo->Updated);

		static const glm::mat4 transform = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(16.0f));

		FlushDrawList();

		Material::SetUBO(0, "u_ViewProjectionMatrix", s_SceneInfo->SceneCamera.ViewProjection); 
		Material::SetUBO(0, "u_InversedViewProjectionMatrix", s_SceneInfo->SceneCamera.InversedViewProjection);
		Material::SetUBO(0, "u_ViewMatrix", s_SceneInfo->SceneCamera.View);
		Material::SetUBO(2, "u_CameraPosition", s_SceneInfo->SceneCamera.CameraPos);

		s_SceneInfo->GridMaterial->SetMat4("u_Transform", transform); //TODO: UBO

		s_SceneInfo->Updated = false;
	}

	void OpenGLSceneRendering::SubmitMesh(const Memory::Shared<Mesh>& mesh, const glm::mat4& transform) const
	{
		s_SceneInfo->MeshDrawList.push_back( { transform, mesh } );
	}

	void OpenGLSceneRendering::SetEnvironment(const Environment& env)
	{
		m_Environment = env;
		s_SceneInfo->SkyboxMaterial->SetImage2D("u_EnvTexture", m_Environment.Radiance);
	}

	Environment OpenGLSceneRendering::CreateEnvironmentScene(const std::filesystem::path& filepath) const
	{
		static Memory::Shared<Shader> equirectangularConversionShader, envFilteringShader, envIrradianceShader, spBRDF;
		if (!equirectangularConversionShader)
			equirectangularConversionShader = Rendering::GetShaderLibrary()->Get("equirect2cube_cs.glsl");

		ImageSpecification spec;

		spec.Width = kEnvMapSize;
		spec.Height = kEnvMapSize;
		spec.Format = ImageFormat::RGBA32F;
		spec.TextureSamples = 1;
		spec.Type = TextureRendererType::TextureCube;
		spec.Data = std::nullopt;

		auto envTextureUnfiltered = Image2D::Create(spec);
	
		Rendering::SubmitCommand([envTextureUnfiltered]()
			{
				envTextureUnfiltered.As<OpenGLImage2D>()->Invalidate();

				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			});
		Memory::Shared<Texture2D> envEquirect = Texture2D::Create(filepath);
		RADIANT_VERIFY(envEquirect->GetImage2D()->GetImageFormat() == ImageFormat::RGBA32F, "Texture is not HDR!");

		equirectangularConversionShader->Use();
		Rendering::SubmitCommand([envEquirect, envTextureUnfiltered]()
			{
				auto id = envEquirect->GetImage2D()->GetTextureID();

				glBindTextureUnit(0, id);
				glBindImageTexture(0, envTextureUnfiltered->GetTextureID(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);
				glDispatchCompute(kEnvMapSize / 32, kEnvMapSize / 32, 6);
				glGenerateTextureMipmap(envTextureUnfiltered->GetTextureID());
			});

		// Compute pre-filtered specular environment map.

		auto envTexture = Image2D::Create(spec);
		
		if(!envFilteringShader)
			envFilteringShader = Shader::Create("Resources/Shaders/spmap_cs.glsl");

		Rendering::SubmitCommand([envTextureUnfiltered, envTexture]()
			{
				envTexture.As<OpenGLImage2D>()->Invalidate();

				glCopyImageSubData(envTextureUnfiltered->GetTextureID(), GL_TEXTURE_CUBE_MAP, 0, 0, 0, 0,
				envTexture->GetTextureID(), GL_TEXTURE_CUBE_MAP, 0, 0, 0, 0,
				kEnvMapSize, kEnvMapSize, 6);

				glUseProgram(envFilteringShader->GetRenderingID());
				glBindTextureUnit(0, envTextureUnfiltered->GetTextureID());

				// Pre-filter rest of the mip chain.
				const float deltaRoughness = 1.0f / glm::max(float(envTexture->GetMipmapLevels() - 1), 1.0f);
				for (int level = 1, size = kEnvMapSize / 2; level <= envTexture->GetMipmapLevels(); ++level, size /= 2) {
					const GLuint numGroups = glm::max(1, size / 32);
					glBindImageTexture(0, envTexture->GetTextureID(), level, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);
					glProgramUniform1f(envFilteringShader->GetRenderingID(), 0, level * deltaRoughness);
					glDispatchCompute(numGroups, numGroups, 6);
				}
			});

		// Compute diffuse irradiance cubemap.

		//NOTE: This way to change image size to load irradiance map
		spec.Width = kIrradianceMapSize;
		spec.Height = kIrradianceMapSize;

		auto irmapTexture = Image2D::Create(spec);

		if(!envIrradianceShader)
			envIrradianceShader = Shader::Create("Resources/Shaders/irmap_cs.glsl");

		Rendering::SubmitCommand([envTexture, irmapTexture]()
			{
				irmapTexture.As<OpenGLImage2D>()->Invalidate();

				glBindTextureUnit(0, envTexture->GetTextureID());
				glBindImageTexture(0, irmapTexture->GetTextureID(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);
				glDispatchCompute(kIrradianceMapSize / 32, kIrradianceMapSize / 32, 6);
			});
		

		return { envTexture , irmapTexture };
	}

	void OpenGLSceneRendering::GeometryPass()
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

			if (diffuse.Enabled)
			{
				s_SceneInfo->RenderPassList.GeoData.material->SetImage2D("u_AlbedoTexture", diffuse.Texture->GetImage2D());
				if (normal.Enabled)
					s_SceneInfo->RenderPassList.GeoData.material->SetImage2D("u_NormalTexture", normal.Texture->GetImage2D());
			}

			if (roughness.Enabled)
				s_SceneInfo->RenderPassList.GeoData.material->SetImage2D("u_RoughnessTexture", roughness.Texture->GetImage2D());
			if (metalness.Enabled)
				s_SceneInfo->RenderPassList.GeoData.material->SetImage2D("u_MetalnessTexture", metalness.Texture->GetImage2D());

			s_SceneInfo->RenderPassList.GeoData.material->SetFloat("u_Roughness", roughness.Roughness);
			s_SceneInfo->RenderPassList.GeoData.material->SetFloat("u_Metalness", metalness.Metalness); 
			s_SceneInfo->RenderPassList.GeoData.material->SetVec3("u_AlbedoColor", diffuse.AlbedoColor);

			//UBO
			Material::SetUBO(2, "u_EnvironmentLight", &s_SceneInfo->LightEnvironment.DirectionalLights, kLightEnvironmentSize);
			Material::SetUBO(2, "u_EnvMapRotation", m_EnvMapRotation);
			Material::SetUBO(2, "u_IBLContribution", m_IBLContribution);

			//Update toggles
			s_SceneInfo->RenderPassList.GeoData.material->SetBool("u_UseNormalTexture", normal.Enabled);
			s_SceneInfo->RenderPassList.GeoData.material->SetBool("u_UseAlbedoTexture", diffuse.Enabled);
			s_SceneInfo->RenderPassList.GeoData.material->SetBool("u_UseMetalnessTexture", metalness.Enabled);
			s_SceneInfo->RenderPassList.GeoData.material->SetBool("u_UseRoughnessTexture", roughness.Enabled);

			// Env. map
			s_SceneInfo->RenderPassList.GeoData.material->SetImage2D("u_EnvRadianceTex", m_Environment.Radiance);
			s_SceneInfo->RenderPassList.GeoData.material->SetImage2D("u_EnvIrradianceTex", m_Environment.Irradiance);
			s_SceneInfo->RenderPassList.GeoData.material->SetImage2D("u_BRDFLUTTexture", s_SceneInfo->BRDF_LUT->GetImage2D());

			//Shadow
			s_SceneInfo->RenderPassList.GeoData.material->SetMat4("u_LightMatrixCascade0", s_SceneInfo->RenderPassList.Shadowdata.LightMatrices[0]);
			s_SceneInfo->RenderPassList.GeoData.material->SetMat4("u_LightMatrixCascade1", s_SceneInfo->RenderPassList.Shadowdata.LightMatrices[1]);
			s_SceneInfo->RenderPassList.GeoData.material->SetMat4("u_LightMatrixCascade2", s_SceneInfo->RenderPassList.Shadowdata.LightMatrices[2]);
			s_SceneInfo->RenderPassList.GeoData.material->SetMat4("u_LightMatrixCascade3", s_SceneInfo->RenderPassList.Shadowdata.LightMatrices[3]);
			s_SceneInfo->RenderPassList.GeoData.material->SetVec4("u_CascadeSplits", s_SceneInfo->RenderPassList.Shadowdata.CascadeSplits);
			s_SceneInfo->RenderPassList.GeoData.material->SetMat4("u_LightView", s_SceneInfo->RenderPassList.Shadowdata.LightViewMatrix);

			s_SceneInfo->RenderPassList.GeoData.material->SetImage2D("u_ShadowMapTexture1", s_SceneInfo->RenderPassList.Shadowdata.ShadowPassPipeline[0]->
				GetSpecification().RenderPass->GetSpecification().TargetFramebuffer->GetDepthAttachmentImage(),
				s_SceneInfo->RenderPassList.Shadowdata.ShadowMapSampler);

			s_SceneInfo->RenderPassList.GeoData.material->SetImage2D("u_ShadowMapTexture2", s_SceneInfo->RenderPassList.Shadowdata.ShadowPassPipeline[1]->
				GetSpecification().RenderPass->GetSpecification().TargetFramebuffer->GetDepthAttachmentImage(),
				s_SceneInfo->RenderPassList.Shadowdata.ShadowMapSampler);

			s_SceneInfo->RenderPassList.GeoData.material->SetImage2D("u_ShadowMapTexture3", s_SceneInfo->RenderPassList.Shadowdata.ShadowPassPipeline[2]->
				GetSpecification().RenderPass->GetSpecification().TargetFramebuffer->GetDepthAttachmentImage(),
				s_SceneInfo->RenderPassList.Shadowdata.ShadowMapSampler);

			s_SceneInfo->RenderPassList.GeoData.material->SetImage2D("u_ShadowMapTexture4", s_SceneInfo->RenderPassList.Shadowdata.ShadowPassPipeline[3]->
				GetSpecification().RenderPass->GetSpecification().TargetFramebuffer->GetDepthAttachmentImage(),
				s_SceneInfo->RenderPassList.Shadowdata.ShadowMapSampler);

			Rendering::SubmitMeshWithMaterial( { mesh.Transform, mesh.Mesh, s_SceneInfo->RenderPassList.GeoData.material }, s_SceneInfo->RenderPassList.GeoData.pipeline);
		}

		if (s_SceneInfo->ShowGrid)
		{
			Rendering::SubmitFullscreenQuad(s_SceneInfo->GridPipeline, s_SceneInfo->GridMaterial);
		}

		Rendering::EndRenderPass();
	}

	void OpenGLSceneRendering::ShadowMapPass()
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

		for(int i = 0; i < 4; i++)
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

	void OpenGLSceneRendering::CompositePass()
	{
		Rendering::BeginRenderPass(s_SceneInfo->RenderPassList.CompData.pipeline->GetSpecification().RenderPass);
		s_SceneInfo->RenderPassList.CompData.material->SetFloat("u_Exposure", m_Scene->GetSceneExposure());
		s_SceneInfo->RenderPassList.CompData.material->SetUint("u_SamplesCount", m_Scene->GetSceneSamplesCount());
		s_SceneInfo->RenderPassList.CompData.material->SetImage2D("u_Texture", s_SceneInfo->RenderPassList.GeoData.pipeline->GetSpecification().RenderPass->GetSpecification().TargetFramebuffer->GetColorAttachmentImage());
		s_SceneInfo->RenderPassList.CompData.pipeline->GetSpecification().Shader->Use();
		Rendering::SubmitFullscreenQuad(s_SceneInfo->RenderPassList.CompData.pipeline, nullptr);
		Rendering::EndRenderPass();
	}

	void OpenGLSceneRendering::FlushDrawList()
	{
		ShadowMapPass(); 
		GeometryPass();
		CompositePass();

		s_SceneInfo->MeshDrawList.clear(); // TODO: Optimize
	}

}