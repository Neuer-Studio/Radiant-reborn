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
					return GL_RGB;
				case ImageFormat::RGBA:
					return GL_RGBA;
				case ImageFormat::RGBA16F:
					return GL_RGBA16F;
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
				return GL_FLOAT;
			case ImageFormat::RGB:
			case ImageFormat::RGBA:
				return GL_UNSIGNED_BYTE;
			}
			RADIANT_VERIFY(false, "Unknown Radiant format");
			return GL_NONE;
		}
	}

	OpenGLImage2D::OpenGLImage2D(ImageSpecification spec)
		: m_Specification(spec)
	{
		m_MipmapLevels = Utils::numMipmapLevels(spec.Width, spec.Height);
	}

	void OpenGLImage2D::Use(uint32_t slot, BindUsage use) const
	{
		auto tID = m_RenderingID;
		Rendering::SubmitCommand([tID, use, slot]() 
			{
				if (use == BindUsage::Unbind)
				{
					glBindTextureUnit(slot, 0);
					return;
				}

				glBindTextureUnit(slot, tID);
			});
	}

	void OpenGLImage2D::Invalidate()
	{
		if (m_RenderingID)
			Release();

		auto texType = Utils::RadiantTexTypeToOGL(m_Specification.Type);
		glGenTextures(1, &m_RenderingID);
		glBindTexture(texType, m_RenderingID);

		auto internalformat = Utils::RadiantInternalFormatToOGL(m_Specification.Format);
		auto format = Utils::RadiantFormatToOGL(m_Specification.Format);
		auto type = Utils::OGLDataType(m_Specification.Format);

		glTextureParameteri(m_RenderingID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RenderingID, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTextureParameteri(m_RenderingID, GL_TEXTURE_MIN_FILTER, m_MipmapLevels > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
		glTextureParameteri(m_RenderingID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameterf(m_RenderingID, GL_TEXTURE_MAX_ANISOTROPY, RenderingAPI::GetGraphicsInfo().MaxAnisotropy);

		if (m_Specification.Data)
			glTexImage2D(texType, 0, internalformat, m_Specification.Width, m_Specification.Height, 0, format, type, m_Specification.Data);
		else
			glTextureStorage2D(m_RenderingID, m_MipmapLevels, internalformat, m_Specification.Width, m_Specification.Height);

		if (m_MipmapLevels > 1)
			glGenerateMipmap(texType);
	}

	void OpenGLImage2D::Release()
	{
		if (!m_RenderingID)
			return;

		Memory::Shared<OpenGLImage2D> instance(this);
		Rendering::SubmitCommand([instance]()
			{
				glDeleteTextures(1, &instance->m_RenderingID);
			});
	}

}