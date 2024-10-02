#pragma once

#include <Radiant/Rendering/Framebuffer.hpp>

namespace Radiant {

	struct RenderPassSpecification
	{
		Common::Memory::Shared<Framebuffer> TargetFramebuffer;
		std::string DebugName;
	};

	class RenderPass : public Common::Memory::RefCounted
	{
	public:
		virtual ~RenderPass() = default;

		virtual RenderPassSpecification& GetSpecification() = 0;
		virtual const RenderPassSpecification& GetSpecification() const = 0;

		static Common::Memory::Shared<RenderPass> Create(const RenderPassSpecification& spec);
	};

}
