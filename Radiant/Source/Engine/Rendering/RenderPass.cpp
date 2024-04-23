#include <Radiant/Rendering/RenderPass.hpp>
#include <Radiant/Rendering/RendererAPI.hpp>

#include <Radiant/Rendering/Platform/OpenGL/OpenGLRenderPass.hpp>

namespace Radiant
{
	Memory::Shared<RenderPass> RenderPass::Create(const RenderPassSpecification& spec)
	{
		switch (RendererAPI::GetAPI())
		{
		case RenderingAPIType::None:    return nullptr;
		case RenderingAPIType::OpenGL:  return Memory::Shared<OpenGLRenderPass>::Create(spec);
		}
		RADIANT_VERIFY(false, "Unknown Rendering API");
		return nullptr;
	}
}