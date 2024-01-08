#pragma once

#include <string>
#include <Radiant/Core/Events/WindowEvents.hpp>

#include <Radiant/Core/Memory/CommandBuffer.hpp>

#include "Window.hpp"

#include <Radiant/Rendering/RenderingAPI.hpp>

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

		template <typename FuncT>
		static void Submit(FuncT&& func)
		{
			auto renderCMD = [](void* ptr)
			{
				auto pFunc = (FuncT*)ptr;

				(*pFunc)();

				pFunc->~FuncT();
			};

			auto storageBuffer = GetCommandBuffer().AddCommand(renderCMD, sizeof(func));
			new (storageBuffer) FuncT(std::forward<FuncT>(func));
		}
	private:
		static Memory::CommandBuffer& GetCommandBuffer();
	private:
		bool OnClose(EventWindowClose& e){}
		void ProcessEvents(Event& e);
	private:
		Memory::Shared<Window> m_Window;

		bool m_Run;
	};


	//Iml. by client
	Application* CreateApplication(int argc, char** argv);
}