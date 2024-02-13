#include <glad/glad.h>

#include <Radiant/Rendering/Platform/OpenGL/OpenGLFramebuffer.hpp>
#include <Radiant/Rendering/Platform/OpenGL/OpenGLImage.hpp>
#include <Radiant/Rendering/Rendering.hpp>

namespace Radiant
{
	namespace Utils
	{
		static GLenum TextureTarget()
		{
			return GL_TEXTURE_2D;
		}
	}

	OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferSpecification& spec)
		: m_Specification(spec)
	{

	}

	void OpenGLFramebuffer::Use(BindUsage usage) const
	{
		Memory::Shared<const OpenGLFramebuffer> instance = this;
		Rendering::SubmitCommand([instance, usage]() mutable
			{
				glBindFramebuffer(GL_FRAMEBUFFER, usage == BindUsage::Bind ? instance->m_RenderingID : 0);
			});
	}

	void OpenGLFramebuffer::Resize(uint32_t width, uint32_t height, bool forceRecreate)
	{
		m_Specification.Width = width;
		m_Specification.Height = height;

		Memory::Shared<OpenGLFramebuffer> instance = this;
		Rendering::SubmitCommand([instance]() mutable
			{
				if (instance->m_RenderingID)
				{
					glDeleteFramebuffers(1, &instance->m_RenderingID);
				}

				glGenFramebuffers(1, &instance->m_RenderingID);
				glBindFramebuffer(GL_FRAMEBUFFER, instance->m_RenderingID);

				instance->m_FbColorImage = Image2D::Create({ instance->m_Specification.Width, instance->m_Specification.Height, instance->m_Specification.Format, TextureRendererType::Texture2D, nullptr });
				instance->m_FbColorImage.As<OpenGLImage2D>()->Invalidate();
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + 0, Utils::TextureTarget(), instance->m_FbColorImage.As<OpenGLImage2D>()->GetTextureID(), 0);

				RADIANT_VERIFY(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete!");

				glBindFramebuffer(GL_FRAMEBUFFER, 0);
			});
	}

}