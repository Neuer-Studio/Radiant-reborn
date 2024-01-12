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
	private:
		GLFWwindow* m_Window;
		int m_GladInit;
	};
}