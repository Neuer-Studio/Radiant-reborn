#include <Radiant/Rendering/Framebuffer.hpp>

#include <Radiant/Rendering/Rendering.hpp>
#include <Radiant/Rendering/Framebuffer.hpp>
#include <Radiant/Rendering/Platform/OpenGL/OpenGLFramebuffer.hpp>

namespace Radiant
{
	Common::Memory::Shared<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec)
	{
		Common::Memory::Shared<Framebuffer> result = nullptr;

		switch (RendererAPI::GetAPI())
		{
			case RenderingAPIType::None:	return nullptr;
			case RenderingAPIType::OpenGL:	result = Common::Memory::Shared<OpenGLFramebuffer>::Create(spec);
		}
		FramebufferPool::Add(result);
		return result;
	}

	void FramebufferPool::Add(Common::Memory::Shared<Framebuffer> framebuffer)
	{
		s_Pool.push_back(framebuffer);
	}

}