#pragma once

#include <GLFW/glfw3.h>

namespace Radiant
{
	class RenderingContext : public Memory::RefCounted
	{
	public:
		virtual ~RenderingContext() = default;

		static Memory::Shared<RenderingContext> Create(GLFWwindow* window);
	};
}