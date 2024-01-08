#pragma once

#include <Radiant/Rendering/RenderingAPI.hpp>
#include <GLFW/glfw3.h>

namespace Radiant
{
	enum class RenderingAPIType : uint8_t
	{
		None = 0,
		Vulkan = 1,
	};

	class RenderingAPI : public Memory::RefCounted
	{
	public:
		static const RenderingAPIType GetAPI();
		static void SetAPI(RenderingAPIType api);
	};
}