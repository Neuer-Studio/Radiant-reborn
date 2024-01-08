#include <Radiant/Rendering/Platform/Vulkan/VulkanRenderingContext.hpp>

#include <Radiant/Rendering/Platform/Vulkan/Vulkan.hpp>

namespace Radiant
{

	static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugReportCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData)
	{
		(void)flags; (void)object; (void)location; (void)messageCode; (void)pUserData; (void)pLayerPrefix; // Unused arguments
		RA_WARN("VulkanDebugCallback:\n  Object Type: {0}\n  Message: {1}", objectType, pMessage);
		return VK_FALSE;
	}

#ifdef RADIANT_CONFIG_DEBUG
	static bool s_DebugValidation = true;
#else 
	static bool s_DebugValidation = false;
#endif
	VulkanRenderingContext::VulkanRenderingContext(GLFWwindow* window)
		: m_Window(window)
	{
		VK_CHECK_RESULT(CreateInstance());

		m_PhysicalDevice = VulkanPhysicalDevice::Create();
		m_LogicalDevice = VulkanDevice::Create(m_PhysicalDevice);
		VulkanSwapChain swap(m_LogicalDevice);
		swap.Create(10, 10);
	}

	VulkanRenderingContext::~VulkanRenderingContext()
	{
		vkDestroyInstance(s_VulkanInstance, nullptr);
	}

	VkResult VulkanRenderingContext::CreateInstance()
	{
		RA_INFO("VulkanRenderingContext::CreateInstance()");
		RADIANT_VERIFY(glfwVulkanSupported(), "GLFW must support Vulkan API");
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Hello Triangle";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		std::vector<const char*> instanceExtensions = {VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME};
		if (s_DebugValidation)
		{
			instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
			instanceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
		}

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = (uint32_t)instanceExtensions.size();
		createInfo.ppEnabledExtensionNames = instanceExtensions.data();
		createInfo.enabledLayerCount = 0;

		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		if (s_DebugValidation)
		{
			const char* validationLayers = "VK_LAYER_KHRONOS_validation";

			uint32_t layerCount;
			vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

			std::vector<VkLayerProperties> availableLayers(layerCount);
			vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

			bool validationLayerPresent = false;
			RA_INFO("Vulkan Instance Layers:");
			for (const VkLayerProperties& layer : availableLayers)
			{
				RA_INFO("  {0}", layer.layerName);
				for (const auto& layerProperties : availableLayers) {
					if (strcmp(layer.layerName, validationLayers) == 0) {
						validationLayerPresent = true;
						break;
					}
				}
			}

			if (validationLayerPresent)
			{
				createInfo.ppEnabledLayerNames = &validationLayers;
				createInfo.enabledLayerCount = 1;
			}
			else
			{
				RA_ERROR("Validation layer VK_LAYER_KHRONOS_validation not present, validation is disabled");
			}
		}

		VkResult result = vkCreateInstance(&createInfo, nullptr, &s_VulkanInstance);
		if (s_DebugValidation)
		{
			auto vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(s_VulkanInstance, "vkCreateDebugReportCallbackEXT");
			RADIANT_VERIFY(vkCreateDebugReportCallbackEXT != NULL, "");
			VkDebugReportCallbackCreateInfoEXT debug_report_ci = {};
			debug_report_ci.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
			debug_report_ci.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
			debug_report_ci.pfnCallback = VulkanDebugReportCallback;
			debug_report_ci.pUserData = NULL;
			VK_CHECK_RESULT(vkCreateDebugReportCallbackEXT(s_VulkanInstance, &debug_report_ci, nullptr, &m_DebugReportCallback));
		}

		return result;
	}
}