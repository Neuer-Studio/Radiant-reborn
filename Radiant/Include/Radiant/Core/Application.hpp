#pragma once

#include <string>

#include "Window.hpp"

namespace Radiant
{ 
	struct ApplicationSpecification
	{
		std::string Name = "TheRock";
		uint32_t WindowWidth = 1600, WindowHeight = 900;
	};

	class Application
	{
	public:
		Application(const ApplicationSpecification& specification);

		void Run();
	private:
		Ref<Window> m_Window;

		bool m_Run;
	};


	//Iml. by client
	Application* CreateApplication(int argc, char** argv);
}