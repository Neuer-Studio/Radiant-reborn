#pragma once

#include <Radiant/Rendering/RenderingContext.hpp>
#include <Radiant/Rendering/Platform/Vulkan/VulkanDevice.hpp>
#include <Radiant/Rendering/Platform/Vulkan/VulkanSwapChain.hpp>
#include <vulkan/vulkan.hpp>

namespace Radiant
{
	class VulkanRenderingContext : public RenderingContext
	{
	public:
		VulkanRenderingContext(GLFWwindow* window);
		virtual ~VulkanRenderingContext() override;

		virtual void BeginFrame() const override {}
		virtual void EndFrame() const override {}

		static const VkInstance GetVulkanInstance() { return s_VulkanInstance; }

		VkResult CreateInstance();
	private:
		VkDebugReportCallbackEXT m_DebugReportCallback = VK_NULL_HANDLE;
		Memory::Shared<VulkanPhysicalDevice> m_PhysicalDevice;
		Memory::Shared<VulkanDevice> m_LogicalDevice;
		Memory::Shared<VulkanSwapChain> m_SwapChain;
		inline static VkInstance s_VulkanInstance;
		GLFWwindow* m_Window;
	};
}