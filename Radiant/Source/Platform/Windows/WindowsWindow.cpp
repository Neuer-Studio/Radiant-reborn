#include "WindowsWindow.hpp"

namespace Radiant
{
	static void GLFWErrorCallback(int error, const char* description)
	{
		RA_ERROR("GLFW Error: ({}: {})", error, description);
	}

	static bool s_GLFWInitialized = false;

	WindowsWindow::WindowsWindow(const WindowSpecification& specification)
		: m_Specification(specification)
	{
		RA_INFO("[Window(Windows)] Creating window {} ({}, {})", m_Specification.Title, m_Specification.Width, m_Specification.Height);

		if (!s_GLFWInitialized)
		{
			// TODO: glfwTerminate on system shutdown
			int success = glfwInit();
			RADIANT_VERIFY (success, "Could not intialize GLFW!");
			glfwSetErrorCallback(GLFWErrorCallback);

			s_GLFWInitialized = true;
		}

		if (!m_Specification.Decorated)
		{
			// This removes titlebar on all platforms
			// and all of the native window effects on non-Windows platforms
#ifdef RADIANT_PLATFORM_WINDOWS
			glfwWindowHint(GLFW_DECORATED, false);
#else
			glfwWindowHint(GLFW_DECORATED, false);
#endif
		}

		m_Window = glfwCreateWindow((int)m_Specification.Width, (int)m_Specification.Height, m_Specification.Title.c_str(), nullptr, nullptr);
		glfwMakeContextCurrent(m_Window);

		//glfwSetWindowUserPointer(m_Window, &m_Data);
	}

	WindowsWindow::~WindowsWindow()
	{

	}

	const std::string& WindowsWindow::GetTitle() const
	{
		return m_Specification.Title;
	}

	void WindowsWindow::SetTitle(const std::string& title)
	{

	}

}