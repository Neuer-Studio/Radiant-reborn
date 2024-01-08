#include <Radiant/Rendering/Platform/Vulkan/VulkanDevice.hpp>
#include <Rendering/Platform/Vulkan/Vulkan.hpp>
#include <Rendering/Platform/Vulkan/VulkanRenderingContext.hpp>

namespace Radiant
{
	VulkanPhysicalDevice::VulkanPhysicalDevice()
	{
		CreateDevice();
	}

	Memory::Shared<VulkanPhysicalDevice> VulkanPhysicalDevice::Create()
	{
		return Memory::Shared< VulkanPhysicalDevice>::Create();
	}

	void VulkanPhysicalDevice::CreateDevice()
	{
		auto instance = VulkanRenderingContext::GetVulkanInstance();
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
		RADIANT_VERIFY(deviceCount, "Failed to find GPUs with Vulkan support!");
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

		VkPhysicalDevice selectedPhysicalDevice = nullptr;
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;

		auto isDeviceSuitable = [&deviceProperties, &deviceFeatures](VkPhysicalDevice device) -> bool // NOTE: Lambda function to check is device suitable
		{
			vkGetPhysicalDeviceProperties(device, &deviceProperties);
			vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

			return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
				deviceFeatures.geometryShader;
		};

		for (const auto& device : devices) {
			if (isDeviceSuitable(device)) {
				selectedPhysicalDevice = device;
				break;
			}
		}

		if (!selectedPhysicalDevice)
		{
			RADIANT_VERIFY("Could not find discrete GPU.");
			selectedPhysicalDevice = devices.back();
		}

		RADIANT_VERIFY(selectedPhysicalDevice, "Could not find any physical devices!");

		m_PhysicalDevice = selectedPhysicalDevice;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, nullptr);

		m_QueueFamilyProperties.resize(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, m_QueueFamilyProperties.data());

		m_QueueFamilyIndices = GetQueueFamilyIndices(VK_QUEUE_GRAPHICS_BIT);

		static constexpr float queuePriority = 1.0f;

		// Graphics queue
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = m_QueueFamilyIndices.graphicsFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		m_QueueCreateInfos.push_back(queueCreateInfo);

		// Check for ext.

		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &extensionCount, nullptr);

		if (extensionCount)
		{
			std::vector<VkExtensionProperties> availableExtensions(extensionCount);
			vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &extensionCount, availableExtensions.data());

			RADIANT_VERIFY("Selected physical device has {0} extensions", extensions.size());
			for (const auto& ext : availableExtensions)
			{
				m_SupportedExtensions.emplace(ext.extensionName);
			}
		}

	}

	Radiant::VulkanPhysicalDevice::QueueFamilyIndices VulkanPhysicalDevice::GetQueueFamilyIndices(int flags)
	{
		QueueFamilyIndices indices;

		int i = 0;
		for (const auto& queueFamily : m_QueueFamilyProperties)
		{
			if (flags & VK_QUEUE_GRAPHICS_BIT) 
			{
				if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
					indices.graphicsFamily = i;
				}
			}

			i++;
		}

		return indices;
	}




	VulkanDevice::VulkanDevice(Memory::Shared<VulkanPhysicalDevice> physicalDevice)
		: m_PhysicalDevice(physicalDevice)
	{
		CreateDevice();
	}

	Radiant::Memory::Shared<Radiant::VulkanDevice> VulkanDevice::Create(Memory::Shared<VulkanPhysicalDevice> physicalDevice)
	{
		return Memory::Shared<VulkanDevice>::Create(physicalDevice);
	}

	void VulkanDevice::CreateDevice()
	{
		VkDeviceCreateInfo createInfo{};
		VkPhysicalDeviceFeatures deviceFeatures{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = m_PhysicalDevice->m_QueueCreateInfos.data();
		createInfo.queueCreateInfoCount = m_PhysicalDevice->m_QueueCreateInfos.size();
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = 0;

		std::vector<const char*> deviceExtensions;
		RADIANT_VERIFY(m_PhysicalDevice->IsExtensionSupported(VK_KHR_SWAPCHAIN_EXTENSION_NAME));
		deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

		if (deviceExtensions.size() > 0)
		{
			createInfo.ppEnabledExtensionNames = deviceExtensions.data();
			createInfo.enabledExtensionCount = (uint32_t)deviceExtensions.size();
		}
		
		VK_CHECK_RESULT(vkCreateDevice(m_PhysicalDevice->GetPhysicalDevice(), &createInfo, nullptr, &m_Device));
	}

}