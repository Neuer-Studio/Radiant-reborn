#pragma once

#include <vulkan/vulkan.h>
#include <Radiant/Rendering/Platform/Vulkan/VulkanDevice.hpp>

namespace Radiant
{
	class VulkanSwapChain : public Memory::RefCounted
	{
	public:
		struct SwapChainSupportDetails 
		{
			VkSurfaceCapabilitiesKHR Capabilities;
			std::vector<VkSurfaceFormatKHR> Formats;
			std::vector<VkPresentModeKHR> PresentModes;
		};
		VulkanSwapChain(const Memory::Shared<VulkanDevice>& device);
		void Create(uint32_t Width, uint32_t Height);
	private:
		VkSurfaceFormatKHR FindSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR FindPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D Find2DExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	private:
		SwapChainSupportDetails m_SwapChainSupportDetails;
		Memory::Shared<VulkanDevice> m_LogicalDevice;
		VkSurfaceKHR m_Surface;
		VkSurfaceFormatKHR m_SurfaceFormat;
		VkPresentModeKHR m_PresentMode;
		VkSwapchainKHR m_SwapChain;
		bool m_IsVsync = false;

		uint32_t m_Width = -1, m_Height = -1;
	};
}