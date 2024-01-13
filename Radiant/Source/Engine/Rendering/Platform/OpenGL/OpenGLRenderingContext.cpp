#include <glad/glad.h>
#include <Radiant/Rendering/Platform/OpenGL/OpenGLRenderingContext.hpp>
#include <Radiant/Rendering/Rendering.hpp>

namespace Radiant
{
	static void OpenGLLogMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
	{
		switch (severity)
		{
		case GL_DEBUG_SEVERITY_HIGH:
			RA_ERROR("[OpenGL Debug HIGH] {0}", message);
			RADIANT_VERIFY(false, "GL_DEBUG_SEVERITY_HIGH");
			break;
		case GL_DEBUG_SEVERITY_MEDIUM:
			RA_WARN("[OpenGL Debug MEDIUM] {0}", message);
			break;
		case GL_DEBUG_SEVERITY_LOW:
			RA_INFO("[OpenGL Debug LOW] {0}", message);
			break;
		case GL_DEBUG_SEVERITY_NOTIFICATION:
			break;
		}
	}

	OpenGLRenderingContext::OpenGLRenderingContext(GLFWwindow* window)
		: m_Window(window)
	{
		m_GladInit = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		RADIANT_VERIFY(m_GladInit, "Failed to initialize OpenGL context");

		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(OpenGLLogMessage, 0);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
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