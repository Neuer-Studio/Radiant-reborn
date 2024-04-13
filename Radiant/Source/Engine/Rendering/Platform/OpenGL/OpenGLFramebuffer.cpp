#include <glad/glad.h>

#include <Radiant/Rendering/Platform/OpenGL/OpenGLFramebuffer.hpp>
#include <Radiant/Rendering/Platform/OpenGL/OpenGLImage.hpp>
#include <Radiant/Rendering/Rendering.hpp>

namespace Radiant
{
	namespace Utils
	{
		static GLenum TextureTarget(bool multisampled)
		{
			return multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
		}

		static void CreateTextures(bool multisampled, RenderingID* outID, uint32_t count)
		{
			glCreateTextures(TextureTarget(multisampled), 1, outID);
		}

		static void BindTexture(bool multisampled, RenderingID id)
		{
			glBindTexture(TextureTarget(multisampled), id);
		}

		static std::optional<RenderingID> GenFramebuffer(uint32_t count = 1)
		{
			RenderingID id;
			glGenFramebuffers(count, &id);

			if (id)
				return id;
			return {};
		}

		static bool IsDepthFormat(ImageFormat format)
		{
			switch (format)
			{
			case ImageFormat::DEPTH24STENCIL8:
			case ImageFormat::DEPTH32F:
				return true;
			}
			return false;
		}

		static Memory::Shared<Image2D> CreateAndAttachColorAttachment(int samples, ImageFormat format, uint32_t width, uint32_t height, int index)
		{
			Memory::Shared<Image2D> image;

			ImageSpecification spec;
			spec.Width = width;
			spec.Height = height;
			spec.Format = format;
			spec.TextureSamples = samples;
			spec.Data = std::nullopt;
			spec.Type = TextureRendererType::Texture2D;

			image = Image2D::Create(spec);
			image.As<OpenGLImage2D>()->Invalidate();

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, Utils::TextureTarget(samples > 1), image->GetTextureID(), 0);
			return image;
		}

		static GLenum DepthAttachmentType(ImageFormat format)
		{
			switch (format)
			{
			case ImageFormat::DEPTH32F:        return GL_DEPTH_ATTACHMENT;
			case ImageFormat::DEPTH24STENCIL8: return GL_DEPTH_STENCIL_ATTACHMENT;
			}
			RADIANT_VERIFY(false, "Unknown format");
			return 0;
		}

		static Memory::Shared<Image2D> CreateAndAttachDepthTexture(int samples, ImageFormat format, uint32_t width, uint32_t height)
		{
			Memory::Shared<Image2D> image;

			ImageSpecification spec;
			spec.Width = width;
			spec.Height = height;
			spec.Format = format;
			spec.TextureSamples = samples;
			spec.Data = std::nullopt;
			spec.Type = TextureRendererType::Texture2D;

			image = Image2D::Create(spec);
			image.As<OpenGLImage2D>()->Invalidate();

			glFramebufferTexture2D(GL_FRAMEBUFFER, Utils::DepthAttachmentType(format), Utils::TextureTarget(samples > 1), image->GetTextureID(), 0);
			return image;
		}

	}

	OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferSpecification& spec)
		: m_Specification(spec)
	{
		RADIANT_VERIFY(spec.Attachments.Attachments.size());

		for (const auto& format : spec.Attachments.Attachments)
		{
			if (!Utils::IsDepthFormat(format))
				m_ColorAttachmentFormats.emplace_back(format);
			else
				m_DepthAttachmentFormat = format;
		}

		uint32_t width = spec.Width;
		uint32_t height = spec.Height;

		Resize(width, height);

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

					for (auto& image : instance->m_ColorAttachments)
					{
						image.As<OpenGLImage2D>()->Release();
					}
					if(instance->m_DepthAttachment.As<OpenGLImage2D>())
						instance->m_DepthAttachment.As<OpenGLImage2D>()->Release();
					instance->m_ColorAttachments.clear();
				}

				instance->m_RenderingID = Utils::GenFramebuffer();
				glBindFramebuffer(GL_FRAMEBUFFER, instance->m_RenderingID.value_or(0));

				if (instance->m_ColorAttachmentFormats.size())
				{
					instance->m_ColorAttachments.resize(instance->m_ColorAttachmentFormats.size());

					// Create color attachments
					for (size_t i = 0; i < instance->m_ColorAttachments.size(); i++)
						instance->m_ColorAttachments[i] = Utils::CreateAndAttachColorAttachment(instance->m_Specification.Samples, instance->m_ColorAttachmentFormats[i], instance->m_Specification.Width, instance->m_Specification.Height, i);
				}

				if (instance->m_DepthAttachmentFormat != ImageFormat::None)
				{
					instance->m_DepthAttachment = Utils::CreateAndAttachDepthTexture(instance->m_Specification.Samples, instance->m_DepthAttachmentFormat, instance->m_Specification.Width, instance->m_Specification.Height);
				}

				RADIANT_VERIFY(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete!");

				glBindFramebuffer(GL_FRAMEBUFFER, 0);
			});
	}

}