#pragma once

#include <Radiant/Rendering/Framebuffer.hpp>

namespace Radiant
{
	class OpenGLFramebuffer : public Framebuffer
	{
	public:
		OpenGLFramebuffer(const FramebufferSpecification& spec);

		virtual const FramebufferSpecification GetFBSpecification() const override { return m_Specification; }
		virtual void Use(BindUsage = BindUsage::Bind) const override;

		virtual void Resize(uint32_t width, uint32_t height, bool forceRecreate = false) override;

		virtual RenderingID GetRendererID() const override { return m_RenderingID.value_or(0); }

		virtual Memory::Shared<Image2D> GetColorAttachmentImage(uint32_t index) const override { return m_ColorAttachments[index]; }
		virtual Memory::Shared<Image2D> GetDepthAttachmentImage() const override { return m_DepthAttachment; }
	private:
		std::optional<RenderingID> m_RenderingID;
		FramebufferSpecification m_Specification;

		std::vector<Memory::Shared<Image2D>> m_ColorAttachments;
		Memory::Shared<Image2D> m_DepthAttachment;

		std::vector<ImageFormat> m_ColorAttachmentFormats;
		ImageFormat m_DepthAttachmentFormat = ImageFormat::None;
	};
}