#include <Glad/glad.h>

#include <Radiant/Rendering/Platform/OpenGL/OpenGLImage.hpp>
#include <Radiant/Rendering/Rendering.hpp>

namespace Radiant
{
	namespace Utils
	{
		static GLuint RadiantInternalFormatToOGL(ImageFormat format)
		{
			switch (format)
			{
				case ImageFormat::RGB:
					return GL_RGB;
				case ImageFormat::RGBA:
					return GL_RGBA;
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
			case ImageFormat::RGBA:
				return GL_RGBA;
			}
			RADIANT_VERIFY(false, "Unknown Radiant format");
			return GL_NONE;
		}

		static GLuint OGLDataType(ImageFormat format)
		{
			switch (format)
			{
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

		Memory::Shared<OpenGLImage2D> instance(this);
		Rendering::SubmitCommand([instance]() mutable	
			{
				glGenTextures(1, &instance->m_RenderingID);
				glBindTexture(GL_TEXTURE_2D, instance->m_RenderingID);

				auto internalformat = Utils::RadiantInternalFormatToOGL(instance->m_Specification.Format);
				auto format = Utils::RadiantFormatToOGL(instance->m_Specification.Format);
				auto type = Utils::OGLDataType(instance->m_Specification.Format);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

				glTexImage2D(GL_TEXTURE_2D, 0, internalformat, instance->m_Specification.Width, instance->m_Specification.Height, 0, format, type, instance->m_Specification.Data);
				glGenerateMipmap(GL_TEXTURE_2D);
			});
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