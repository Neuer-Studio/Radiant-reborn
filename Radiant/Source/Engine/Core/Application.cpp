#include <Core/Application.hpp>
#include <GLFW/glfw3.h>
#include <Radiant/Rendering/Rendering.hpp>
#include <Radiant/Rendering/Framebuffer.hpp>

namespace Radiant
{
	Application* Application::s_Instance = nullptr;
	static Memory::Shared<RenderingContext> s_RenderingContext = nullptr;

	Application::Application(const ApplicationSpecification& specification)
	{
		if (s_Instance)
		{
			RADIANT_VERIFY(false);
		}

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

		s_Instance = this;

		m_ImGuiLayer = ImGuiLayer::Create("ImGuiLayer");
		PushLayer(m_ImGuiLayer);
	}

	Application::~Application()
	{
		for (Layer* layer : m_LayerStack)
		{
			layer->OnDetach();
			delete layer;
		}
	}

	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PopLayer(Layer* layer)
	{
		m_LayerStack.PopLayer(layer);
		layer->OnDetach();
	}

	void Application::Run()
	{
		OnInit();

		Rendering::GetRenderingCommandBuffer().Execute();
		while (m_Run)
		{
			for (Layer* layer : m_LayerStack)
				layer->OnUpdate();

			s_RenderingContext->BeginFrame();
			Rendering::GetRenderingCommandBuffer().Execute();	

			m_ImGuiLayer->Begin();

			for (Layer* layer : m_LayerStack)
				layer->OnImGuiRender();

			m_ImGuiLayer->End();

			s_RenderingContext->EndFrame();
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
				s_RenderingContext->OnResize(e.width, e.height);

				auto& fbs = FramebufferPool::GetAll();

				for (auto& fb : fbs)
					fb->Resize(e.width, e.height);
				

				return true;
			});
	}
}