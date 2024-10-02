#pragma once

#include <Radiant/Rendering/RenderingTypes.hpp>
#include <Radiant/Rendering/Image.hpp>

namespace Radiant
{
	struct FramebufferAttachmentSpecification
	{
		FramebufferAttachmentSpecification() = default;
		FramebufferAttachmentSpecification(const std::initializer_list<ImageFormat>& attachments)
			: Attachments(attachments) {}

		std::vector<ImageFormat> Attachments;
	};

	struct FramebufferSpecification
	{
		uint32_t Width;
		uint32_t Height;
		uint32_t Samples = 2; // Multisampling
		FramebufferAttachmentSpecification Attachments;

		bool NoResizeble = false;
	};

	class Framebuffer : public Common::Memory::RefCounted
	{
	public:
		virtual ~Framebuffer() = default;

		virtual const FramebufferSpecification GetFBSpecification() const = 0;
		virtual void Use(BindUsage = BindUsage::Bind) const = 0;

		virtual void Resize(uint32_t width, uint32_t height, bool forceRecreate = false) = 0;

		virtual RenderingID GetRendererID() const = 0;

		virtual Common::Memory::Shared<Image2D> GetColorAttachmentImage(uint32_t index = 0) const = 0;
		virtual Common::Memory::Shared<Image2D> GetDepthAttachmentImage() const = 0;

		static Common::Memory::Shared<Framebuffer> Create(const FramebufferSpecification& spec);
	};

	class FramebufferPool final
	{
	public:
		static void Add(Common::Memory::Shared<Framebuffer> framebuffer);

		static std::vector<Common::Memory::Shared<Framebuffer>>& GetAll() { return s_Pool; }
	private:
		static inline std::vector<Common::Memory::Shared<Framebuffer>> s_Pool;
	};
}