#pragma once

#include <Common/Core/Result.hpp>

#include <Radiant/Rendering/Platform/Vulkan/VulkanDevice.hpp>
#include <Radiant/Rendering/Platform/Vulkan/BufferTypes.hpp>

namespace Radiant::Rendering::Vulkan
{
	
	class VulkanBuffer final
	{
	public: 
		VulkanBuffer() = default;
	public:

		Common::Result<VkResult> RT_CreateBuffer(const Radiant::VulkanDevice& device, const BufferTypes& bufferType, uint32_t size);
	};
}