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

		static std::optional<RenderingID> GenFramebuffer(uint32_t count = 1)
		{
			RenderingID id;
			glGenFramebuffers(count, &id);

			if (id)
				return id;
			return {};
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
				glBindFramebuffer(GL_FRAMEBUFFER, usage == BindUsage::Bind ? instance->m_RenderingID.value_or(0) : 0);
				glViewport(0, 0, instance->m_Specification.Width, instance->m_Specification.Height);
			});
	}

	void OpenGLFramebuffer::Resize(uint32_t width, uint32_t height, bool forceRecreate)
	{
		if (forceRecreate)
			return;

		m_Specification.Width = width;
		m_Specification.Height = height;

		Memory::Shared<OpenGLFramebuffer> instance = this;
		Rendering::SubmitCommand([instance]() mutable
			{
				if (instance->m_RenderingID.has_value())
				{
					glDeleteFramebuffers(1, &instance->m_RenderingID.value());
					instance->m_RenderingID.reset();

					auto id = instance->GetColorImage()->GetTextureID();
					glDeleteTextures(1, &id);
				}

				instance->m_RenderingID = Utils::GenFramebuffer();
				glBindFramebuffer(GL_FRAMEBUFFER, instance->m_RenderingID.value_or(0));

				instance->m_FbColorImage = Image2D::Create({ instance->m_Specification.Width, instance->m_Specification.Height, instance->m_Specification.Format, TextureRendererType::Texture2D, {} });
				instance->m_FbColorImage.As<OpenGLImage2D>()->Invalidate();
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + 0, Utils::TextureTarget(), instance->m_FbColorImage.As<OpenGLImage2D>()->GetTextureID(), 0);

				RADIANT_VERIFY(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete!");

				glBindFramebuffer(GL_FRAMEBUFFER, 0);
			});
	}

}