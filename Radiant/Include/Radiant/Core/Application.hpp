#pragma once

#include <string>
#include <Radiant/Core/Events/WindowEvents.hpp>
#include <Radiant/Core/Events/MouseEvents.hpp>
#include <Radiant/Core/Timestep.hpp>

#include "Window.hpp"

#include <Radiant/Rendering/RendererAPI.hpp>
#include "LayerStack.hpp"
#include <Radiant/ImGui/ImGuiLayer.hpp>

namespace Radiant
{ 
	struct ApplicationSpecification
	{
		std::string Name = "TheRock";
		uint32_t WindowWidth = 1600, WindowHeight = 900;
		bool Fullscreen = false;
		RenderingAPIType APIType = RenderingAPIType::OpenGL;
	};

	class Application
	{
	public:
		Application(const ApplicationSpecification& specification);
		void Run();

		virtual ~Application();

		virtual void OnInit() = 0;
		virtual void OnShutdown() = 0;
		virtual void OnUpdate(Timestep ts) = 0;

		void PushLayer(Layer* layer);
		void PopLayer(Layer* layer);

		const Memory::Shared<Window>& GetWindow() const { return m_Window;}

	public:
		static Application& GetInstance() { return *s_Instance; }
	private:
	private:
		bool OnClose(EventWindowClose& e){}
		void ProcessEvents(Event& e);
	private:
		Memory::Shared<Window> m_Window;
		LayerStack m_LayerStack;
		ImGuiLayer* m_ImGuiLayer;
	private:
		static Application* s_Instance;
		Timestep m_Timestep;
		float m_LastFrameTime = 0.0f;
		uint32_t m_FrameCount = 0;

		bool m_Run;
	};


	//Iml. by client
	Application* CreateApplication(int argc, char** argv);
}