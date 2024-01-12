#include <glad/glad.h>
#include <Radiant/Rendering/Platform/OpenGL/OpenGLRenderingContext.hpp>
#include <Radiant/Rendering/Rendering.hpp>

namespace Radiant
{
	OpenGLRenderingContext::OpenGLRenderingContext(GLFWwindow* window)
		: m_Window(window)
	{
		m_GladInit = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		RADIANT_VERIFY(m_GladInit, "Failed to initialize OpenGL context");
	}

	void OpenGLRenderingContext::BeginFrame() const
	{
		glfwSwapBuffers(m_Window);
		glfwPollEvents();
	}

	void OpenGLRenderingContext::EndFrame() const
	{
		static float rgba[4] = { 0.4f, 0.3f, 0.1f, 1.0f };
		Rendering::Clear(rgba);
	}

}