#include <Radiant/Core/Input.hpp>
#include <Radiant/Core/Application.hpp>

#include <GLFW/glfw3.h>

namespace Radiant::Input
{
	bool Keyboard::IsKeyPressed(KeyCode key)
	{
		const GLFWwindow* window = static_cast <const GLFWwindow*>(Application::GetInstance().GetWindow()->GetNativeWindow());
		auto state = glfwGetKey((GLFWwindow*)window, static_cast<int32_t>(key));

		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	bool Mouse::IsMouseButtonPressed(MouseButton button)
	{
		auto window = static_cast<const GLFWwindow*>(Application::GetInstance().GetWindow()->GetNativeWindow());
		auto state = glfwGetMouseButton((GLFWwindow*)window, static_cast<int32_t>(button));

		return state == GLFW_PRESS;
	}

	float Mouse::GetMouseX()
	{
		auto [x, y] = GetMousePosition();
		return (float)x;
	}

	float Mouse::GetMouseY()
	{
		auto [x, y] = GetMousePosition();
		return (float)y;
	}

	std::pair<float, float> Mouse::GetMousePosition()
	{
		auto window = static_cast<const GLFWwindow*>(Application::GetInstance().GetWindow()->GetNativeWindow());
		double x, y;
		glfwGetCursorPos((GLFWwindow*)window, &x, &y);
		return { (float)x, (float)y };
	}

	Mouse& Mouse::Get()
	{
		static Mouse s_Instance;
		return s_Instance;
	}

	void Mouse::SetCursorMode(MouseState mode)
	{
		m_MouseMode = mode;

		auto& window = Application::GetInstance().GetWindow();
		glfwSetInputMode((GLFWwindow*)(window->GetNativeWindow()), GLFW_CURSOR, GLFW_CURSOR_NORMAL + (int)mode);
	}

}