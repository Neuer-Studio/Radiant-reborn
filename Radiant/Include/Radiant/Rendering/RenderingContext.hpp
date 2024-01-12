#pragma once

#include <GLFW/glfw3.h>

namespace Radiant
{
	class RenderingContext : public Memory::RefCounted
	{
	public:
		virtual ~RenderingContext() = default;

		virtual void BeginFrame() const = 0;
		virtual void EndFrame() const = 0;

		static Memory::Shared<RenderingContext> Create(GLFWwindow* window);
	};
}