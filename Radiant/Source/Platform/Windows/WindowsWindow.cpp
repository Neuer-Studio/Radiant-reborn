#include "WindowsWindow.hpp"

#include <Radiant/Core/Application.hpp>
#include <Radiant/Core/Events/Event.hpp>
#include <Radiant/Core/Events/WindowEvents.hpp>
#include <Radiant/Rendering/Rendering.hpp>
#include <Radiant/Rendering/RenderingAPI.hpp>

namespace Radiant
{
	static void GLFWErrorCallback(int error, const char* description)
	{
		RA_ERROR("GLFW Error: ({}: {})", error, description);
	}

	static bool s_GLFWInitialized = false;

	WindowsWindow::WindowsWindow(const WindowSpecification& specification)
	{
		m_Data.Specification = specification;
		RA_INFO("[Window(Windows)] Creating window {} ({}, {})", m_Data.Specification.Title, m_Data.Specification.Width, m_Data.Specification.Height);

		if (!s_GLFWInitialized)
		{
			// TODO: glfwTerminate on system shutdown
			int success = glfwInit();
			RADIANT_VERIFY (success, "Could not intialize GLFW!");
			glfwSetErrorCallback(GLFWErrorCallback);

			s_GLFWInitialized = true;
		}

		if(RenderingAPI::GetAPI() == RenderingAPIType::Vulkan)
		{ 
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		}

		if (!m_Data.Specification.Decorated)
		{
			// This removes titlebar on all platforms
			// and all of the native window effects on non-Windows platforms
#ifdef RADIANT_PLATFORM_WINDOWS
			glfwWindowHint(GLFW_DECORATED, false);
#else
			glfwWindowHint(GLFW_DECORATED, false);
#endif
		}

		m_Window = glfwCreateWindow((int)m_Data.Specification.Width, (int)m_Data.Specification.Height, m_Data.Specification.Title.c_str(), nullptr, nullptr);
		glfwMakeContextCurrent(m_Window);
		glfwSetWindowUserPointer(m_Window, &m_Data);

		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
			{
				auto& data = *(WindowData*)glfwGetWindowUserPointer(window);

				EventWindowClose event;
				data.EventCallback(event);
			});

		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
			{
				auto& data = *(WindowData*)glfwGetWindowUserPointer(window);
				EventWindowResize event((unsigned int)width, (unsigned int)height);
				data.EventCallback(event);
			});


		auto context = Rendering::Initialize(m_Window);
	}

	WindowsWindow::~WindowsWindow()
	{

	}

	const std::string& WindowsWindow::GetTitle() const
	{
		return m_Data.Specification.Title;
	}

	void WindowsWindow::SetTitle(const std::string& title)
	{

	}

	void WindowsWindow::SetSize(uint32_t width, uint32_t height)
	{
		m_Data.Specification.Width = width;
		m_Data.Specification.Height = height;
	}

	bool WindowsWindow::IsWindowMaximized() const
	{
		return (bool)glfwGetWindowAttrib(m_Window, GLFW_MAXIMIZED);
	}

}