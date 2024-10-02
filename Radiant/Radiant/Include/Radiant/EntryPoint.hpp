#pragma once

extern Radiant::Application* Radiant::CreateApplication(int argc, char** argv);

int main(int arc, char** argv)
{
	Radiant::LogInit();
	auto app = Radiant::CreateApplication(arc, argv);
	app->Run();
	delete app;

	std::exit(0);

	return 0;
}