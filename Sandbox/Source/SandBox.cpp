#include <Radiant/Radiant.hpp>
#include "EditorLayer.hpp"

class Sandbox : public Radiant::Application
{
public:
	Sandbox(const Radiant::ApplicationSpecification& props)
		: Radiant::Application(props)
	{

	}

	virtual void OnInit() override
	{
		PushLayer(new Radiant::EditorLayer());
	}

	virtual void OnShutdown() override {}
	virtual void OnUpdate(Radiant::Timestep ts) override {}
	
};

Radiant::Application* Radiant::CreateApplication(int argc, char** argv)
{
	Radiant::ApplicationSpecification specification;
	specification.Name = "Title";
	specification.APIType = RenderingAPIType::OpenGL;
	specification.Fullscreen = false;
	specification.WindowWidth = 1280;
	specification.WindowHeight = 720;
	return new Sandbox(specification);
}