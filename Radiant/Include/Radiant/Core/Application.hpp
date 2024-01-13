#pragma once

#include <string>
#include <Radiant/Core/Events/WindowEvents.hpp>

#include "Window.hpp"

#include <Radiant/Rendering/RenderingAPI.hpp>
#include "LayerStack.hpp"

namespace Radiant
{ 
	struct ApplicationSpecification
	{
		std::string Name = "TheRock";
		uint32_t WindowWidth = 1600, WindowHeight = 900;
		RenderingAPIType APIType = RenderingAPIType::Vulkan;
	};

	class Application
	{
	public:
		Application(const ApplicationSpecification& specification);
		void Run();

		virtual ~Application();

		virtual void OnInit() {}
		virtual void OnShutdown() {}
		virtual void OnUpdate() {}

		void PushLayer(Layer* layer);
		void PopLayer(Layer* layer);
	private:
	private:
		bool OnClose(EventWindowClose& e){}
		void ProcessEvents(Event& e);
	private:
		Memory::Shared<Window> m_Window;
		LayerStack m_LayerStack;

		bool m_Run;
	};


	//Iml. by client
	Application* CreateApplication(int argc, char** argv);
}