#include <glad/glad.h>
#include <glm/glm.hpp>

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
		Memory::Shared<Shader> DefaultShader;
		Memory::Shared<Shader> FullscreenQuadShader;
		Memory::Shared<Material> FullscreenQuadMaterial;

		struct RenderPassList RenderPassList;

		std::vector<DrawCommand> MeshDrawList;
	};

	static SceneInfo* s_SceneInfo = nullptr;

	void OpenGLSceneRendering::Init()
	{
		m_ViewportWidth = Application::GetInstance().GetWindow()->GetWidth();
		m_ViewportHeight = Application::GetInstance().GetWindow()->GetHeight();
		s_SceneInfo = new SceneInfo();

		s_SceneInfo->FullscreenQuadShader = Shader::Create("Resources/Shaders/Skybox.glsl");
		s_SceneInfo->FullscreenQuadMaterial = Material::Create(s_SceneInfo->FullscreenQuadShader);

		// Geometry pass

		{
			RenderPassSpecification renderPassSpec;
			renderPassSpec.TargetFramebuffer = Framebuffer::Create({ m_ViewportWidth, m_ViewportHeight, 1, ImageFormat::RGBA16F });
			renderPassSpec.DebugName = "Geometry Render Pass";

			PipelineSpecification pipelineSpecification;
			pipelineSpecification.Layout = {
				{ ShaderDataType::Float3, "a_Position" },
				{ ShaderDataType::Float2, "a_TexCoord" }
			};

			pipelineSpecification.DebugName = "PBR-Static";
			pipelineSpecification.RenderPass = RenderPass::Create(renderPassSpec);
			pipelineSpecification.Shader = Shader::Create("Resources/Shaders/StaticPBR_Radiant.glsl");

			s_SceneInfo->RenderPassList.GeoData.pipeline = Pipeline::Create(pipelineSpecification);
		}

		// Composite pass

		{
			RenderPassSpecification renderPassSpec;
			renderPassSpec.TargetFramebuffer = Framebuffer::Create({ m_ViewportWidth, m_ViewportHeight, 1, ImageFormat::RGBA16F });
			renderPassSpec.DebugName = "Composite Render Pass";

			PipelineSpecification pipelineSpecification;
			pipelineSpecification.Layout = {
				{ ShaderDataType::Float3, "a_Position" },
				{ ShaderDataType::Float2, "a_TexCoord" }
			};

			pipelineSpecification.DebugName = "Scene Composite";
			pipelineSpecification.RenderPass = RenderPass::Create(renderPassSpec);
			pipelineSpecification.Shader = Shader::Create("Resources/Shaders/SceneComposite.glsl");
			s_SceneInfo->RenderPassList.CompData.pipeline = Pipeline::Create(pipelineSpecification);

			s_SceneInfo->RenderPassList.CompData.material = Material::Create(pipelineSpecification.Shader);
		}

		s_SceneInfo->MeshDrawList.reserve(100); // TODO: Add a capcaity from YAML(scene)
	}

	Memory::Shared<Radiant::Image2D> OpenGLSceneRendering::GetFinalPassImage() const
	{
		return s_SceneInfo->RenderPassList.CompData.pipeline->GetSpecification().RenderPass->GetSpecification().TargetFramebuffer->GetColorImage();
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

	OpenGLSceneRendering::~OpenGLSceneRendering()
	{
		delete s_SceneInfo;
	}

	OpenGLSceneRendering::OpenGLSceneRendering(const Memory::Shared<Scene>& scene)
		: m_Scene(scene)
	{
		Init();
	}

	void OpenGLSceneRendering::OnUpdate(Timestep ts, Camera& cam)
	{
		cam.OnUpdate(ts);

		auto viewProjection = cam.GetProjectionMatrix() * cam.GetViewMatrix();
		s_SceneInfo->FullscreenQuadMaterial->SetUniform("TransformUniforms", "u_ViewProjectionMatrix", glm::inverse(viewProjection));
		Flush();
	}

	void OpenGLSceneRendering::SubmitMesh(const Memory::Shared<Mesh>& mesh, const glm::mat4& transform) const
	{
		s_SceneInfo->MeshDrawList.push_back({ transform, mesh, nullptr });
	}

	void OpenGLSceneRendering::SetEnvironment(const Environment& env)
	{
		m_Environment = env;
		s_SceneInfo->FullscreenQuadMaterial->SetUniform("u_EnvTexture", m_Environment.Radiance); 
	}

	Environment OpenGLSceneRendering::CreateEnvironmentScene(const std::filesystem::path& filepath) const
	{
		static Memory::Shared<Shader> equirectangularConversionShader, envFilteringShader, envIrradianceShader, spBRDF;
		if (!equirectangularConversionShader)
			equirectangularConversionShader = Shader::Create("Resources/Shaders/equirect2cube_cs.glsl");

		auto envTextureUnfiltered = Image2D::Create({ kEnvMapSize, kEnvMapSize,ImageFormat::RGBA16F, TextureRendererType::TextureCube,nullptr });
	
		Rendering::SubmitCommand([envTextureUnfiltered]()
			{
				envTextureUnfiltered.As<OpenGLImage2D>()->Invalidate();

				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			});
		Memory::Shared<Texture2D> envEquirect = Texture2D::Create(filepath);
		RADIANT_VERIFY(envEquirect->GetImage2D()->GetImageFormat() == ImageFormat::RGBA16F, "Texture is not HDR!");

		equirectangularConversionShader->Use();
		Rendering::SubmitCommand([envEquirect, envTextureUnfiltered]()
			{
				auto id = envEquirect->GetImage2D()->GetTextureID();

				glBindTextureUnit(0, id);
				glBindImageTexture(0, envTextureUnfiltered->GetTextureID(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
				glDispatchCompute(kEnvMapSize / 32, kEnvMapSize / 32, 6);
				glGenerateTextureMipmap(envTextureUnfiltered->GetTextureID());
			});

		// Compute pre-filtered specular environment map.

		auto envTexture = Image2D::Create({ kEnvMapSize, kEnvMapSize,ImageFormat::RGBA16F, TextureRendererType::TextureCube,nullptr });
		
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
					glBindImageTexture(0, envTexture->GetTextureID(), level, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
					glProgramUniform1f(envFilteringShader->GetRenderingID(), 0, level * deltaRoughness);
					glDispatchCompute(numGroups, numGroups, 6);
				}
			});

		// Compute diffuse irradiance cubemap.

		auto irmapTexture = Image2D::Create({ kIrradianceMapSize, kIrradianceMapSize,ImageFormat::RGBA16F, TextureRendererType::TextureCube,nullptr });

		if(!envIrradianceShader)
			envIrradianceShader = Shader::Create("Resources/Shaders/irmap_cs.glsl");

		Rendering::SubmitCommand([envTexture, irmapTexture]()
			{
				irmapTexture.As<OpenGLImage2D>()->Invalidate();

				glBindTextureUnit(0, envTexture->GetTextureID());
				glBindImageTexture(0, irmapTexture->GetTextureID(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
				glDispatchCompute(kIrradianceMapSize / 32, kIrradianceMapSize / 32, 6);
			});
		

		return { envTexture , irmapTexture };
	}

	void OpenGLSceneRendering::GeometryPass()
	{
		s_SceneInfo->RenderPassList.GeoData.pipeline->GetSpecification().RenderPass->GetSpecification().TargetFramebuffer->Use();
		s_SceneInfo->FullscreenQuadShader->Use();
		Rendering::DrawFullscreenQuad(s_SceneInfo->RenderPassList.GeoData.pipeline);

		//Shader::Use();
		//Pipeline::Use();

		for (const auto& mesh : s_SceneInfo->MeshDrawList) {
			//Rendering::DrawMesh(mesh);
		}

		s_SceneInfo->RenderPassList.GeoData.pipeline->GetSpecification().RenderPass->GetSpecification().TargetFramebuffer->Use(BindUsage::Unbind);
	}

	void OpenGLSceneRendering::CompositePass()
	{
		s_SceneInfo->RenderPassList.CompData.pipeline->GetSpecification().RenderPass->GetSpecification().TargetFramebuffer->Use();
		s_SceneInfo->RenderPassList.CompData.pipeline->GetSpecification().Shader->Use();
		s_SceneInfo->RenderPassList.CompData.material->SetUniform("Uniforms", "Exposure", 1.0f);
		s_SceneInfo->RenderPassList.CompData.material->SetUniform("u_Texture", s_SceneInfo->RenderPassList.GeoData.pipeline->GetSpecification().RenderPass->GetSpecification().TargetFramebuffer->GetColorImage());
		Rendering::DrawFullscreenQuad(s_SceneInfo->RenderPassList.CompData.pipeline);
		s_SceneInfo->RenderPassList.CompData.pipeline->GetSpecification().RenderPass->GetSpecification().TargetFramebuffer->Use(BindUsage::Unbind);
	}
	
	void OpenGLSceneRendering::Flush() 
	{
		GeometryPass();
		CompositePass();
	}

}