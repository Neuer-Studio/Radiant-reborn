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
		RADIANT_VERIFY(!s_Instance, "it is not possible to create more than one instance");

		RendererAPI::SetAPI(specification.APIType);
		s_Instance = this;

		WindowSpecification wspec;
		wspec.Width = specification.WindowWidth;
		wspec.Height = specification.WindowHeight;
		wspec.Title = specification.Name;
		wspec.Fullscreen = specification.Fullscreen;

		m_Window = Window::Create(wspec);
		m_Window->SetEventCallback([this](Event& e)
			{
				this->ProcessEvents(e);
			});

		s_RenderingContext = Rendering::GetRenderingContext();


		s_RenderingContext->OnResize(m_Window->GetWidth(), m_Window->GetHeight());

		m_ImGuiLayer = ImGuiLayer::Create("ImGuiLayer");
		Rendering::GetRenderingCommandBuffer().Execute();

		PushLayer(m_ImGuiLayer);

		m_Window->Maximize();
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
			m_FrameCount = 0;
			s_RenderingContext->BeginFrame();

			if (!m_Window->IsWindowMinimized())
			{
				for (Layer* layer : m_LayerStack)
					layer->OnUpdate(m_Timestep);

				Rendering::GetRenderingCommandBuffer().Execute();

				m_ImGuiLayer->Begin();

				for (Layer* layer : m_LayerStack)
					layer->OnImGuiRender();

				m_ImGuiLayer->End();
			}

			s_RenderingContext->EndFrame();

			float time = glfwGetTime();
			m_Timestep = time - m_LastFrameTime;
			m_LastFrameTime = time;
		}
		OnShutdown();
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
				if (m_Window->IsWindowMinimized())
					return false;
				s_RenderingContext->OnResize(e.width, e.height);

				auto& fbs = FramebufferPool::GetAll();

				for (auto& fb : fbs)
				{
					if(!fb->GetFBSpecification().NoResizeble)
						fb->Resize(e.width, e.height);
				}

				return false;
			});

		for (const auto& layer : m_LayerStack)
		{
			layer->OnEvent(e);
			if (e.m_Handled)
				break;
		}
	}
}