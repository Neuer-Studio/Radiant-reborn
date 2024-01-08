#pragma once

#include <vulkan/vulkan.hpp>

namespace Radiant
{
#define VK_CHECK_RESULT(f)											             \
{																	             \
	VkResult res = (f);												             \
	if (res != VK_SUCCESS)											             \
	{																             \
		RA_ERROR("VkResult is 'NONE' in {1}:{2}", __FILE__ , __LINE__); \
		RADIANT_VERIFY(res == VK_SUCCESS);										 \
	}																			 \
}
}