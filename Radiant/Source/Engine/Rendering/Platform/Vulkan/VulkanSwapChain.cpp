#include <Radiant/Rendering/Platform/Vulkan/VulkanSwapChain.hpp>
#include <Radiant/Rendering/Platform/Vulkan/Vulkan.hpp>

namespace Radiant
{
	VulkanSwapChain::VulkanSwapChain(const Memory::Shared<VulkanDevice>& device)
		: m_LogicalDevice(device)
	{
	}

	void VulkanSwapChain::Create(uint32_t Width, uint32_t Height)
	{
		m_Width = Width;
		m_Height = Height;

		const VkPhysicalDevice& physicalDevice = m_LogicalDevice->GetPhysicalDevice();

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, m_Surface, &m_SwapChainSupportDetails.Capabilities);
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_Surface, &formatCount, nullptr);
		RADIANT_VERIFY(formatCount);

		if (formatCount != 0) {
			m_SwapChainSupportDetails.Formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_Surface, &formatCount, m_SwapChainSupportDetails.Formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_Surface, &presentModeCount, nullptr);

		if (presentModeCount != 0) {
			m_SwapChainSupportDetails.PresentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, m_Surface, &presentModeCount, m_SwapChainSupportDetails.PresentModes.data());
		}
		RADIANT_VERIFY(presentModeCount);

		m_SurfaceFormat = FindSurfaceFormat(m_SwapChainSupportDetails.Formats);
		m_PresentMode = FindPresentMode(m_SwapChainSupportDetails.PresentModes);
		VkExtent2D swapchainExtent = Find2DExtent(m_SwapChainSupportDetails.Capabilities);

		uint32_t imageCount = m_SwapChainSupportDetails.Capabilities.minImageCount + 1;

		if (m_SwapChainSupportDetails.Capabilities.maxImageCount > 0 && imageCount > m_SwapChainSupportDetails.Capabilities.maxImageCount) {
			imageCount = m_SwapChainSupportDetails.Capabilities.maxImageCount;
		}
		
		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = m_Surface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = m_SurfaceFormat.format;
		createInfo.imageColorSpace = m_SurfaceFormat.colorSpace;
		createInfo.imageExtent = swapchainExtent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		createInfo.preTransform = m_SwapChainSupportDetails.Capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = m_PresentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		VK_CHECK_RESULT(vkCreateSwapchainKHR(m_LogicalDevice->GetLogicalDevice(), &createInfo, nullptr, &m_SwapChain));
	}

	VkSurfaceFormatKHR VulkanSwapChain::FindSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		for (const auto& availableFormat : availableFormats) 
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return availableFormat;
			}
		}
		return VkSurfaceFormatKHR{ VK_FORMAT_B8G8R8A8_UNORM , availableFormats[0].colorSpace };
	}

	VkPresentModeKHR VulkanSwapChain::FindPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
	{
		if (m_IsVsync)
			return VK_PRESENT_MODE_FIFO_KHR;
		for (const auto& availablePresentMode : availablePresentModes) {
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return availablePresentMode;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D VulkanSwapChain::Find2DExtent(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.currentExtent.width != (uint32_t)-1)
		{
			m_Width = capabilities.currentExtent.width;
			m_Height = capabilities.currentExtent.height;
			return capabilities.currentExtent;
		}

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(m_Width),
			static_cast<uint32_t>(m_Height)
		};

		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}

}