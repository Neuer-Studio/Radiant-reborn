#pragma once

#include <Windows.h>

#include <Core/Window.hpp>
#include <GLFW/glfw3.h>

namespace Radiant
{
	class WindowsWindow : public Window
	{
	public:
		WindowsWindow(const WindowSpecification& specification);
		~WindowsWindow() override;

		virtual const std::string& GetTitle() const override;
		virtual void SetTitle(const std::string& title) override;
	private:
		WindowSpecification m_Specification;

		GLFWwindow* m_Window;
	};
}