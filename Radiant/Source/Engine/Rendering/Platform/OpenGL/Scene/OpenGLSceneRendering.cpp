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
#include <Radiant/Rendering/Platform/OpenGL/OpenGLImage.hpp>

namespace Radiant
{
	static constexpr int kEnvMapSize = 2048;
	static constexpr int kIrradianceMapSize = 32;
	static constexpr int kBRDF_LUT_Size = 256;

	struct GeometryData
	{
		Memory::Shared<Pipeline> pipeline;
		Memory::Shared<Material> material;
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
	};

	struct DrawCommand
	{
		glm::mat4 Transform;
		Memory::Shared<Mesh> Mesh;
		Memory::Shared<Material> Material;
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
			renderPassSpec.TargetFramebuffer = Framebuffer::Create({ m_ViewportWidth, m_ViewportHeight, 1, { ImageFormat::RGBA16F, ImageFormat::DEPTH24STENCIL8 } });
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
			pipelineSpecification.Shader = Rendering::GetShaderLibrary()->Get("SceneComposite.glsl");
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

			s_SceneInfo->GridMaterial->LoadUniformToBuffer("u_Transform", RadiantShaderType::Vertex, RadiantShaderDataType::Float4);
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

		s_SceneInfo->MeshDrawList.reserve(100); // TODO: Add a capcaity from YAML(scene)
	}

	Memory::Shared<Radiant::Image2D> OpenGLSceneRendering::GetFinalPassImage() const
	{
		return s_SceneInfo->RenderPassList.CompData.pipeline->GetSpecification().RenderPass->GetSpecification().TargetFramebuffer->GetColorAttachmentImage(0);
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

	void OpenGLSceneRendering::UpdateCamera(const Camera& camera)
	{
		s_SceneInfo->SceneCamera.ViewProjection = camera.GetViewProjection();
		s_SceneInfo->SceneCamera.View = camera.GetViewMatrix();
		s_SceneInfo->SceneCamera.Projection = camera.GetProjectionMatrix();
		s_SceneInfo->SceneCamera.InversedViewProjection = glm::inverse(camera.GetViewProjection());

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

		s_SceneInfo->GridMaterial->SetUBO(0, "u_ViewProjectionMatrix", s_SceneInfo->SceneCamera.ViewProjection); 
		s_SceneInfo->GridMaterial->SetUBO(0, "u_InversedViewProjectionMatrix", s_SceneInfo->SceneCamera.InversedViewProjection); 
		s_SceneInfo->GridMaterial->SetMat4("u_Transform", transform);

		s_SceneInfo->Updated = false;
	}

	void OpenGLSceneRendering::SubmitMesh(const Memory::Shared<Mesh>& mesh, const glm::mat4& transform) const
	{
		s_SceneInfo->MeshDrawList.push_back({ transform, mesh, nullptr });
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

		auto envTextureUnfiltered = Image2D::Create({ kEnvMapSize, kEnvMapSize,ImageFormat::RGBA32F, TextureRendererType::TextureCube,nullptr });
	
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

		auto envTexture = Image2D::Create({ kEnvMapSize, kEnvMapSize,ImageFormat::RGBA32F, TextureRendererType::TextureCube,nullptr });
		
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

		auto irmapTexture = Image2D::Create({ kIrradianceMapSize, kIrradianceMapSize,ImageFormat::RGBA32F, TextureRendererType::TextureCube,nullptr });

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
			s_SceneInfo->RenderPassList.GeoData.material->SetUBO(1, "u_Transform", mesh.Transform);

			const auto& diffuse = mesh.Mesh->GetMaterialDiffuseData();
			if (diffuse.Enabled)
			{
				s_SceneInfo->RenderPassList.GeoData.material->SetBool("u_DiffuseTextureEnabled", true);
				s_SceneInfo->RenderPassList.GeoData.material->SetImage2D("u_DiffuseTexture", diffuse.Texture->GetImage2D());
			}

			s_SceneInfo->RenderPassList.GeoData.material->Use(); // NOTE: Using shader
			Rendering::SubmitMesh(mesh.Mesh, s_SceneInfo->RenderPassList.GeoData.pipeline);
		}

		if (s_SceneInfo->ShowGrid)
		{
			Rendering::SubmitFullscreenQuad(s_SceneInfo->GridPipeline, s_SceneInfo->GridMaterial);
		}

		Rendering::EndRenderPass();
	}

	void OpenGLSceneRendering::CompositePass()
	{
		Rendering::BeginRenderPass(s_SceneInfo->RenderPassList.CompData.pipeline->GetSpecification().RenderPass);
		//s_SceneInfo->RenderPassList.CompData.material->SetUniform("Uniforms", "Exposure", 1.0f);
		s_SceneInfo->RenderPassList.CompData.material->SetImage2D("u_Texture", s_SceneInfo->RenderPassList.GeoData.pipeline->GetSpecification().RenderPass->GetSpecification().TargetFramebuffer->GetColorAttachmentImage());
		s_SceneInfo->RenderPassList.CompData.pipeline->GetSpecification().Shader->Use();
		Rendering::SubmitFullscreenQuad(s_SceneInfo->RenderPassList.CompData.pipeline, nullptr);
		Rendering::EndRenderPass();
	}
	
	void OpenGLSceneRendering::FlushDrawList()
	{
		GeometryPass();
		CompositePass();

		s_SceneInfo->MeshDrawList.clear(); // TODO: Optimize
	}

}