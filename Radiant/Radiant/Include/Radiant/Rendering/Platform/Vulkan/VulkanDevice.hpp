#pragma once

#include <vulkan/vulkan.h>

namespace Radiant
{
	class VulkanPhysicalDevice : public Common::Memory::RefCounted
	{
	public:
		struct QueueFamilyIndices {
			std::optional<int32_t> GraphicsFamily;
			std::optional<int32_t> PresentFamily;
		};
		VulkanPhysicalDevice();

		static Common::Memory::Shared<VulkanPhysicalDevice> Create();
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

	class VulkanDevice : public Common::Memory::RefCounted
	{
	public:
		VulkanDevice(Common::Memory::Shared<VulkanPhysicalDevice> physicalDevice);

		const VkPhysicalDevice& GetPhysicalDevice() const { return m_PhysicalDevice->GetPhysicalDevice(); }
		const VkDevice& GetLogicalDevice() const { return m_LogicalDevice; }

		static Common::Memory::Shared<VulkanDevice> Create(Common::Memory::Shared<VulkanPhysicalDevice> physicalDevice);
		void Destroy();
	private:
		void CreateDevice();
	private:
		Common::Memory::Shared <VulkanPhysicalDevice> m_PhysicalDevice;
		VkDevice m_LogicalDevice;
	};
}