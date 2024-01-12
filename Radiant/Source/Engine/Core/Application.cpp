#include <Core/Application.hpp>
#include <GLFW/glfw3.h>
#include <Radiant/Rendering/Rendering.hpp>

namespace Radiant
{
	static Memory::Shared<RenderingContext> s_RenderingContext = nullptr;

	Application::Application(const ApplicationSpecification& specification)
	{
		RenderingAPI::SetAPI(specification.APIType);

		WindowSpecification wspec;
		wspec.Width = specification.WindowWidth;
		wspec.Height = specification.WindowHeight;
		wspec.Title = specification.Name;

		m_Window = Window::Create(wspec);
		m_Window->SetEventCallback([this](Event& e)
			{
				this->ProcessEvents(e);
			});

		s_RenderingContext = Rendering::GetRenderingContext();
	}

	void Application::Run()
	{
		while (m_Run)
		{
			s_RenderingContext->BeginFrame();
			s_RenderingContext->EndFrame();
			Rendering::GetRenderingCommandBuffer().Execute();		
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
}