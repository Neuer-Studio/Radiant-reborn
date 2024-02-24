#include <Glad/glad.h>

#include <Radiant/Rendering/Platform/OpenGL/OpenGLImage.hpp>
#include <Radiant/Rendering/Rendering.hpp>

namespace Radiant
{
	namespace Utils
	{
		static uint32_t numMipmapLevels(uint32_t width, uint32_t height)
		{
			uint32_t levels = 1;
			while ((width | height) >> levels) {
				++levels;
			}
			return levels;
		}

		static GLuint RadiantInternalFormatToOGL(ImageFormat format)
		{
			switch (format)
			{
				case ImageFormat::RGB:
					return GL_RGB8;
				case ImageFormat::RGBA:
					return GL_RGBA8;
				case ImageFormat::RGBA16F:
					return GL_RGBA16F; 
				case ImageFormat::RGBA32F:
					return GL_RGBA32F;
				case ImageFormat::DEPTH32F:
					return GL_DEPTH_COMPONENT32F;
				case ImageFormat::DEPTH24STENCIL8:
					return GL_DEPTH24_STENCIL8;
			}
			RADIANT_VERIFY(false, "Unknown Radiant format");
			return GL_NONE;
		}

		static GLuint RadiantFormatToOGL(ImageFormat format)
		{
			switch (format)
			{
				case ImageFormat::RGB:
					return GL_RGB;
				case ImageFormat::RGBA16F:
				case ImageFormat::RGBA32F:
				case ImageFormat::RGBA:
					return GL_RGBA;

			}
			RADIANT_VERIFY(false, "Unknown Radiant format");
			return GL_NONE;
		}

		static GLuint RadiantTexTypeToOGL(TextureRendererType type)
		{
			switch (type)
			{
			case TextureRendererType::Texture2D:
				return GL_TEXTURE_2D;
			case TextureRendererType::TextureCube:
				return GL_TEXTURE_CUBE_MAP;
			}
			RADIANT_VERIFY(false, "Unknown Radiant texture type");
			return GL_NONE;
		}

		static GLuint OGLDataType(ImageFormat format)
		{
			switch (format)
			{
			case ImageFormat::RGBA16F:
			case ImageFormat::RGBA32F:
				return GL_FLOAT;
			case ImageFormat::RGB:
			case ImageFormat::RGBA:
				return GL_UNSIGNED_BYTE;

			}
			RADIANT_VERIFY(false, "Unknown Radiant format");
			return GL_NONE;
		}
	}

	OpenGLImage2D::OpenGLImage2D(const ImageSpecification& spec)
		: m_Specification(spec)
	{
		m_MipmapLevels = Utils::numMipmapLevels(spec.Width, spec.Height);
	}

	void OpenGLImage2D::Use(uint32_t slot, BindUsage use) const
	{
		auto tID = m_RenderingID;
		auto sID = m_SamplerRendererID;
		Rendering::SubmitCommand([tID, use, slot, sID]() 
			{
				if (use == BindUsage::Unbind)
				{
					glBindTextureUnit(slot, 0);
					return;
				}

				glBindSampler(slot, sID);
				glBindTextureUnit(slot, tID);
			});
	}

	void OpenGLImage2D::Invalidate()
	{
		if (m_RenderingID != 0)
			Release();

		auto texType = Utils::RadiantTexTypeToOGL(m_Specification.Type);
		glCreateTextures(texType, 1, &m_RenderingID);

		auto internalformat = Utils::RadiantInternalFormatToOGL(m_Specification.Format);
		glTextureStorage2D(m_RenderingID, m_MipmapLevels, internalformat, m_Specification.Width, m_Specification.Height);

		if (m_Specification.Data)
		{ 
			auto format = Utils::RadiantFormatToOGL(m_Specification.Format);
			auto type = Utils::OGLDataType(m_Specification.Format);
			glTextureSubImage2D(m_RenderingID, 0, 0, 0, m_Specification.Width, m_Specification.Height, format, type, m_Specification.Data);
			glGenerateTextureMipmap(m_RenderingID); // TODO: optional
		}
		glCreateSamplers(1, &m_SamplerRendererID);
		glSamplerParameteri(m_SamplerRendererID, GL_TEXTURE_MIN_FILTER, m_Specification.Data ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
		glSamplerParameteri(m_SamplerRendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glSamplerParameteri(m_SamplerRendererID, GL_TEXTURE_WRAP_R, GL_REPEAT);
		glSamplerParameteri(m_SamplerRendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glSamplerParameteri(m_SamplerRendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	void OpenGLImage2D::Release()
	{
		glDeleteTextures(1, &m_RenderingID);
	}

}