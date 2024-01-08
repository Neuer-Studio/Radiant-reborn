#include <Radiant/Rendering/RenderingContext.hpp>
#include <Radiant/Rendering/RenderingAPI.hpp>
#include <Radiant/Rendering/Platform/Vulkan/VulkanRenderingContext.hpp>

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
		}

		RADIANT_VERIFY(false);
		return nullptr;
	}

}