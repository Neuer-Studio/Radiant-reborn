#include <Core/Application.hpp>
#include <GLFW/glfw3.h>

namespace Radiant
{
	static Memory::CommandBuffer s_CommandBuffer;

	Application::Application(const ApplicationSpecification& specification)
	{
		m_Window = Window::Create({});
		m_Window->SetEventCallback([this](Event& e)
			{
				this->ProcessEvents(e);
			});

		RenderingAPI::SetAPI(specification.APIType);
	}

	void Application::Run()
	{
		while (m_Run)
		{
			s_CommandBuffer.Execute();

			glfwSwapBuffers((GLFWwindow*)m_Window->GetNativeWindow());
			glfwPollEvents();
		}
	}

	void Application::ProcessEvents(Event& e)
	{
		EventManager eventManager(e);
		eventManager.Notify<EventWindowClose>([this](const EventWindowClose& e) -> bool
			{
				m_Run = false;
				return true;
			});

		eventManager.Notify<EventWindowResize>([this](const EventWindowResize& e) -> bool
			{
				m_Window->SetSize(e.width, e.height);
				return true;
			});
	}

	Memory::CommandBuffer& Application::GetCommandBuffer()
	{
		return s_CommandBuffer;
	}
}