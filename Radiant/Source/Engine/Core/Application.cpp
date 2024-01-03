#include <Core/Application.hpp>

namespace Radiant
{
	Application::Application(const ApplicationSpecification& specification)
	{
		m_Window = Window::Create({});
	}

	void Application::Run()
	{
		while (m_Run)
		{

		}
	}
}