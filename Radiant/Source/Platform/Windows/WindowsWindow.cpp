#include "WindowsWindow.hpp"

#include <Radiant/Core/Application.hpp>
#include <Radiant/Core/Events/Event.hpp>
#include <Radiant/Core/Events/WindowEvents.hpp>
#include <Radiant/Core/Events/KeyEvents.hpp>
#include <Radiant/Core/Events/MouseEvents.hpp>
#include <Radiant/Rendering/Rendering.hpp>
#include <Radiant/Rendering/RendererAPI.hpp>

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

		if (!s_GLFWInitialized)
		{
			// TODO: glfwTerminate on system shutdown
			int success = glfwInit();
			RADIANT_VERIFY (success, "Could not intialize GLFW!");
			glfwSetErrorCallback(GLFWErrorCallback);

			s_GLFWInitialized = true;
		}

		if(RendererAPI::GetAPI() == RenderingAPIType::Vulkan)
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

		auto primaryMonitor = glfwGetPrimaryMonitor();
		if (specification.Fullscreen)
		{
			auto videoMode = glfwGetVideoMode(primaryMonitor);

			glfwWindowHint(GLFW_DECORATED, false);


			m_Data.Specification.Width = videoMode->width;
			m_Data.Specification.Height = videoMode->height;
			m_Window = glfwCreateWindow(videoMode->width, videoMode->height, m_Data.Specification.Title.c_str(), primaryMonitor, nullptr);
		}

		else
		{
			m_Window = glfwCreateWindow((int)m_Data.Specification.Width, (int)m_Data.Specification.Height, m_Data.Specification.Title.c_str(), nullptr, nullptr);
		/*	int max_width = GetSystemMetrics(SM_CXSCREEN);
			int max_hieght = GetSystemMetrics(SM_CYSCREEN);

			glfwSetWindowMonitor(m_Window, NULL, (max_width / 2) - (m_Data.Specification.Width / 2), (max_hieght / 2) - (m_Data.Specification.Height / 2), m_Data.Specification.Width, m_Data.Specification.Height, GLFW_DONT_CARE);*/
		}

		RA_INFO("[Window(Windows)] Creating window {} ({}, {})", m_Data.Specification.Title, m_Data.Specification.Width, m_Data.Specification.Height);

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

		glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset)
			{
				auto& data = *(WindowData*)glfwGetWindowUserPointer(window);

				MouseScrolledEvent event((float)xOffset, (float)yOffset);
				data.EventCallback(event);
			});

		glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
			{
				auto& data = *((WindowData*)glfwGetWindowUserPointer(window));

				switch (action)
				{
				case GLFW_PRESS:
				{
					KeyPressedEvent event((KeyCode)key, 0);
					data.EventCallback(event);
					break;
				}
				/*case GLFW_RELEASE:
				{
					KeyReleasedEvent event((KeyCode)key);
					data.EventCallback(event);
					break;
				}
				case GLFW_REPEAT:
				{
					KeyPressedEvent event((KeyCode)key, 1);
					data.EventCallback(event);
					break;
				}*/
				}
			});

		glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods)
			{
				auto& data = *((WindowData*)glfwGetWindowUserPointer(window));

				switch (action)
				{
				case GLFW_PRESS:
				{
					MouseButtonPressedEvent event((MouseButton)button);
					data.EventCallback(event);
					break;
				}
				}
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
		m_Data.Specification.Title = title;
	}

	void WindowsWindow::SetSize(uint32_t width, uint32_t height)
	{
		m_Data.Specification.Width = width;
		m_Data.Specification.Height = height;

		glfwSetWindowSize(m_Window, width, height);
	}

	bool WindowsWindow::IsWindowMaximized() const
	{
		return (bool)glfwGetWindowAttrib(m_Window, GLFW_MAXIMIZED);
	}


	void WindowsWindow::Maximize() const
	{
		glfwMaximizeWindow(m_Window);
	}

}