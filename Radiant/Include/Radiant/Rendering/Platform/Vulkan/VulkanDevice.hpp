#pragma once

#include <vulkan/vulkan.h>

namespace Radiant
{
	class VulkanPhysicalDevice : public Memory::RefCounted
	{
	public:
		struct QueueFamilyIndices {
			int32_t graphicsFamily = -1;
			int32_t presentFamily = -1;
		};
		VulkanPhysicalDevice();

		static Memory::Shared<VulkanPhysicalDevice> Create();
		const VkPhysicalDevice& GetPhysicalDevice() const { return m_PhysicalDevice; }
		bool IsExtensionSupported(const std::string& extensionName) const { return m_SupportedExtensions.find(extensionName) != m_SupportedExtensions.end(); }
	private:
		QueueFamilyIndices GetQueueFamilyIndices(int flags);
		void CreateDevice();
	private:
		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
		std::vector<VkQueueFamilyProperties> m_QueueFamilyProperties;
		std::vector<VkDeviceQueueCreateInfo> m_QueueCreateInfos;
		QueueFamilyIndices m_QueueFamilyIndices;

		std::unordered_set<std::string> m_SupportedExtensions;
	private:
		friend class VulkanDevice;
	};

	class VulkanDevice : public Memory::RefCounted
	{
	public:
		VulkanDevice(Memory::Shared<VulkanPhysicalDevice> physicalDevice);

		const VkPhysicalDevice& GetPhysicalDevice() const { return m_PhysicalDevice->GetPhysicalDevice(); }
		const VkDevice& GetLogicalDevice() const { return m_Device; }

		static Memory::Shared<VulkanDevice> Create(Memory::Shared<VulkanPhysicalDevice> physicalDevice);
	private:
		void CreateDevice();
	private:
		Memory::Shared <VulkanPhysicalDevice> m_PhysicalDevice;
		VkDevice m_Device;
	};
}