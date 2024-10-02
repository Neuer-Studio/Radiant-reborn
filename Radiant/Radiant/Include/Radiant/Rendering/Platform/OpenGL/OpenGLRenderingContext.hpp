#pragma once

#include <Radiant/Rendering/RenderingContext.hpp>

namespace Radiant
{
	class OpenGLRenderingContext : public RenderingContext
	{
	public:
		OpenGLRenderingContext(GLFWwindow* window);

		virtual void BeginFrame() const override;
		virtual void EndFrame() const override;

		virtual void Shutdown() override;

		virtual void OnResize(uint32_t width, uint32_t height) override;
	private:
		GLFWwindow* m_Window;
		int m_GladInit;
	};
}