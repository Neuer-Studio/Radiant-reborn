#include <Radiant/Radiant.hpp>
#include <Radiant/Core/Application.hpp>

class Sandbox : public Radiant::Application
{
public:
	Sandbox(const Radiant::ApplicationSpecification& props)
		: Radiant::Application(props)
	{
		
	}
};

Radiant::Application* Radiant::CreateApplication(int argc, char** argv)
{
	Radiant::ApplicationSpecification specification;
	specification.WindowHeight = 1600;
	specification.WindowWidth = 1600;
	specification.Name = "Title";
	specification.APIType = RenderingAPIType::Vulkan;
	return new Sandbox(specification);
}