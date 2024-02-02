#include <Radiant/Rendering/Framebuffer.hpp>

#include <Radiant/Rendering/Rendering.hpp>
#include <Radiant/Rendering/Framebuffer.hpp>
#include <Radiant/Rendering/Platform/OpenGL/OpenGLFramebuffer.hpp>

namespace Radiant
{
	Memory::Shared<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec)
	{
		Memory::Shared<Framebuffer> result = nullptr;

		switch (RenderingAPI::GetAPI())
		{
			case RenderingAPIType::None:	return nullptr;
			case RenderingAPIType::OpenGL:	result = Memory::Shared<OpenGLFramebuffer>::Create(spec);
		}
		FramebufferPool::Add(result);
		return result;
	}

	void FramebufferPool::Add(Memory::Shared<Framebuffer> framebuffer)
	{
		s_Pool.push_back(framebuffer);
	}

}