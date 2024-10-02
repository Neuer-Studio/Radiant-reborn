#pragma once

#include <GLFW/glfw3.h>

namespace Radiant
{
	class RenderingContext : public Common::Memory::RefCounted
	{
	public:
		virtual ~RenderingContext() = default;

		virtual void BeginFrame() const = 0;
		virtual void EndFrame() const = 0;

		virtual void OnResize(uint32_t width, uint32_t height) = 0;
		virtual void Shutdown() = 0;

		static Common::Memory::Shared<RenderingContext> Create(GLFWwindow* window);
	};
}