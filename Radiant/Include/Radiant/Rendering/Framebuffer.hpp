#pragma once

#include <Radiant/Rendering/RenderingTypes.hpp>
#include <Radiant/Rendering/Image.hpp>

namespace Radiant
{
	struct FramebufferSpecification
	{
		uint32_t Width;
		uint32_t Height;
		uint32_t Samples = 1;
		ImageFormat Format;
	};

	class Framebuffer : public Memory::RefCounted
	{
	public:
		virtual ~Framebuffer() = default;

		virtual const FramebufferSpecification GetFBSpecification() const = 0;
		virtual void Use(BindUsage = BindUsage::Bind) const = 0;

		virtual void Resize(uint32_t width, uint32_t height, bool forceRecreate = false) = 0;

		virtual RenderingID GetRendererID() const = 0;

		virtual Memory::Shared<Image2D> GetColorImage() const = 0;

		static Memory::Shared<Framebuffer> Create(const FramebufferSpecification& spec);
	};

	class FramebufferPool final
	{
	public:
		static void Add(Memory::Shared<Framebuffer> framebuffer);

		static std::vector<Memory::Shared<Framebuffer>>& GetAll() { return s_Pool; }
	private:
		static inline std::vector<Memory::Shared<Framebuffer>> s_Pool;
	};
}