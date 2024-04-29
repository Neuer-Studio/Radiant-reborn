#include <glad/glad.h>

#include <Radiant/Rendering/Platform/OpenGL/OpenGLRenderer.hpp>
#include <Radiant/Rendering/Platform/OpenGL/OpenGLImage.hpp>
#include <Radiant/Rendering/Rendering.hpp>

namespace Radiant
{
	static constexpr int kEnvMapSize = 2048;
	static constexpr int kIrradianceMapSize = 32;

	void OpenGLRenderer::Clear(float rgba[4]) const
	{
			float r = (*rgba);
			rgba++;
			float g = (*rgba);
			rgba++;
			float b = (*rgba);
			rgba++;
			float a = (*rgba);
			glClearColor(r, g, b, a);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void OpenGLRenderer::DrawPrimitive(Primitives primitive, uint32_t count, bool depthTest) const
	{
		if (depthTest)
			glEnable(GL_DEPTH_TEST);
		else 
			glDisable(GL_DEPTH_TEST);

		GLenum gltype = 0;
		switch (primitive)
		{
		case Primitives::Triangle:
			gltype = GL_TRIANGLES;
			break;
		case Primitives::Line:
			gltype = GL_LINES;
			break;
		}

		glDrawElements(gltype, count, GL_UNSIGNED_INT, nullptr);
	}

	void OpenGLRenderer::SetLineWidth(float width) const
	{
		glLineWidth(width);
	}

	const Environment OpenGLRenderer::CreateEnvironmentMap(const std::filesystem::path& filepath) const
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

		if (!envFilteringShader)
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

		if (!envIrradianceShader)
			envIrradianceShader = Shader::Create("Resources/Shaders/irmap_cs.glsl");

		Rendering::SubmitCommand([envTexture, irmapTexture]()
			{
				irmapTexture.As<OpenGLImage2D>()->Invalidate();

				glBindTextureUnit(0, envTexture->GetTextureID());
				glBindImageTexture(0, irmapTexture->GetTextureID(), 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);
				glDispatchCompute(kIrradianceMapSize / 32, kIrradianceMapSize / 32, 6);
			});


		return { filepath.string(), envTexture , irmapTexture };
	}

}