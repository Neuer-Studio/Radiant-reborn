#include <Radiant/Rendering/RenderingContext.hpp>
#include <Radiant/Rendering/RenderingAPI.hpp>
#include <Radiant/Rendering/Platform/Vulkan/VulkanRenderingContext.hpp>
#include <Radiant/Rendering/Platform/OpenGL/OpenGLRenderingContext.hpp>

namespace Radiant
{
	Memory::Shared<RenderingContext> RenderingContext::Create(GLFWwindow* window)
	{
		switch(RenderingAPI::GetAPI())
		{
			case RenderingAPIType::Vulkan:
			{
				return Memory::Shared<VulkanRenderingContext>::Create(window);
			}

			case RenderingAPIType::OpenGL:
			{
				return Memory::Shared<OpenGLRenderingContext>::Create(window);
			}
		}

		RADIANT_VERIFY(false);
		return nullptr;
	}

}