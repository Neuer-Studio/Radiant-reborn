#include <glad/glad.h>
#include <Radiant/Rendering/Platform/OpenGL/OpenGLRenderingContext.hpp>
#include <Radiant/Rendering/Rendering.hpp>
#include <Radiant/Core/Application.hpp>

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
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		glFrontFace(GL_CCW);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_MULTISAMPLE);
		glDebugMessageCallback(OpenGLLogMessage, 0);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		auto& info = RenderingAPI::GetGraphicsInfo();

		info.Vendor = (const char*)glGetString(GL_VENDOR);
		info.Renderer = (const char*)glGetString(GL_RENDERER);
		info.Version = (const char*)glGetString(GL_VERSION);

		size_t pos = info.Renderer.find('/');

		if (pos != std::string::npos) 
			info.Renderer = info.Renderer.substr(0, pos);
		

		glGetIntegerv(GL_MAX_SAMPLES, &info.MaxSamples);
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &info.MaxAnisotropy);

		glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &info.MaxTextureUnits);

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

	void OpenGLRenderingContext::OnResize(uint32_t width, uint32_t height)
	{
		Rendering::SubmitCommand([width, height]()
			{
				glViewport(0, 0, width, height);
			});
	}

}