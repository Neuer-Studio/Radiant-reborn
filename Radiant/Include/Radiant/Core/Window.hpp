#pragma once

namespace Radiant
{
	struct WindowSpecification
	{
		std::string Title = "Sandbox";
		uint32_t Width = 1600;
		uint32_t Height = 900;
		bool Decorated = true;
		bool Fullscreen = false;
		bool VSync = true;
	};

	class Window
	{
	public:
		virtual ~Window() {};

		virtual const std::string& GetTitle() const = 0;
		virtual void SetTitle(const std::string& title) = 0;

		static Ref<Window> Create(const WindowSpecification& specification);
	};
}