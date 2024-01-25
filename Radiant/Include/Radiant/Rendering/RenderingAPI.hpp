#pragma once

#include <Radiant/Rendering/RenderingAPI.hpp>
#include <GLFW/glfw3.h>

namespace Radiant
{
	enum class RenderingAPIType : uint8_t
	{
		None = 0,
		Vulkan = 1,
		OpenGL = 2
	};

	enum class Primitives
	{
		Triangle
	};

	class RenderingAPI : public Memory::RefCounted
	{
	public:
		virtual void Clear(float rgba[4]) const = 0;
		virtual void DrawPrimitive(Primitives primitive = Primitives::Triangle, uint32_t count = 3) const = 0;
	public:
		static const RenderingAPIType GetAPI();
		static void SetAPI(RenderingAPIType api);
	};
}