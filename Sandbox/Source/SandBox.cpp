#include <Radiant/Radiant.hpp>

class Sandbox : public Radiant::Application
{
public:
	Sandbox(const Radiant::ApplicationSpecification& props)
		: Radiant::Application(props)
	{}
};

Radiant::Application* Radiant::CreateApplication(int argc, char** argv)
{
	Radiant::ApplicationSpecification specification;
	specification.WindowHeight = 1600;
	specification.WindowWidth = 1600;
	specification.Name = "Title";
	return new Sandbox(specification);
}