#include <glad/glad.h>
#include <glm/glm.hpp>

#include <Radiant/Rendering/Platform/OpenGL/Scene/OpenGLSceneRendering.hpp>
#include <Radiant/Rendering/Image.hpp>
#include <Radiant/Rendering/Rendering.hpp>
#include <Radiant/Rendering/Framebuffer.hpp>
#include <Radiant/Rendering/Pipeline.hpp>
#include <Radiant/Rendering/Platform/OpenGL/OpenGLImage.hpp>

namespace Radiant
{
	static constexpr int kEnvMapSize = 1024;
	static constexpr int kIrradianceMapSize = 32;
	static constexpr int kBRDF_LUT_Size = 256;

	struct GeometryData
	{
		Memory::Shared<Shader> GeometryShader;
		Memory::Shared<Framebuffer> GeometryPass;
		Memory::Shared<Pipeline> GeometryPipeline;
	};

	struct SceneInfo
	{
		GeometryData GeoData;
	};

	static SceneInfo* s_SceneInfo = nullptr;

	void OpenGLSceneRendering::Init()
	{
		s_SceneInfo = new SceneInfo();
	}

	OpenGLSceneRendering::~OpenGLSceneRendering()
	{
		delete s_SceneInfo;
	}

	OpenGLSceneRendering::OpenGLSceneRendering(const std::string& sceneName)
		: m_SceneName(sceneName)
	{
		Init();
	}

	void OpenGLSceneRendering::SubmitScene() const
	{
		s_SceneInfo->GeoData.GeometryPass->Use();



		s_SceneInfo->GeoData.GeometryPass->Use(BindUsage::Unbind);
	}

	Environment OpenGLSceneRendering::CreateEnvironmentScene(const std::filesystem::path& filepath) const
	{
		static Memory::Shared<Shader> equirectangularConversionShader, envFilteringShader, envIrradianceShader, spBRDF;
		if (!equirectangularConversionShader)
			equirectangularConversionShader = Shader::Create("Resources/Shaders/equirect2cube_cs.glsl");

		auto envTextureUnfiltered = Image2D::Create({ kEnvMapSize, kEnvMapSize,ImageFormat::RGBA16F, TextureRendererType::TextureCube,nullptr });
		envTextureUnfiltered.As<OpenGLImage2D>()->Invalidate();
		Rendering::SubmitCommand([envTextureUnfiltered]()
			{
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
		envTexture.As<OpenGLImage2D>()->Invalidate();
		
		if(!envFilteringShader)
			envFilteringShader = Shader::Create("Resources/Shaders/spmap_cs.glsl");

		Rendering::SubmitCommand([envTextureUnfiltered, envTexture]()
			{
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
		irmapTexture.As<OpenGLImage2D>()->Invalidate();

		if(!envIrradianceShader)
			envIrradianceShader = Shader::Create("Resources/Shaders/irmap_cs.glsl");

		Rendering::SubmitCommand([envTexture, irmapTexture]()
			{
				glBindTextureUnit(0, envTexture->GetTextureID());
				glBindImageTexture(0, irmapTexture->GetTextureID(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA16F);
				glDispatchCompute(kIrradianceMapSize / 32, kIrradianceMapSize / 32, 6);
			});

		// Compute Cook-Torrance BRDF 2D LUT for split-sum approximation.

		auto spBRDF_LUT = Image2D::Create({ kBRDF_LUT_Size, kBRDF_LUT_Size,ImageFormat::RGBA16F, TextureRendererType::Texture2D,nullptr });
		spBRDF_LUT.As<OpenGLImage2D>()->Invalidate();

		if(!spBRDF)
			spBRDF = Shader::Create("Resources/Shaders/spbrdf_cs.glsl");

		Rendering::SubmitCommand([spBRDF_LUT]()
			{
				glTextureParameteri(spBRDF_LUT->GetTextureID(), GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTextureParameteri(spBRDF_LUT->GetTextureID(), GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

				glUseProgram(spBRDF->GetRenderingID());
				glBindImageTexture(0, spBRDF_LUT->GetTextureID(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG16F);
				glDispatchCompute(kBRDF_LUT_Size / 32, kBRDF_LUT_Size / 32, 1);

				glFinish();
			});

		return { envTexture , irmapTexture };
	}
}