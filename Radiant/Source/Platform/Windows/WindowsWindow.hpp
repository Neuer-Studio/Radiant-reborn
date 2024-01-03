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

		virtual void SetSize(uint32_t width, uint32_t height) override;
		virtual uint32_t GetWidth() const { return m_Data.Specification.Width; }
		virtual uint32_t GetHeight() const { return m_Data.Specification.Height; }

		virtual void SetEventCallback(const EventCallbackFn& e) override { m_Data.EventCallback = e; }
	private:
		struct WindowData
		{
			WindowSpecification Specification;
			EventCallbackFn EventCallback;
		} m_Data;

		GLFWwindow* m_Window;
	};
}