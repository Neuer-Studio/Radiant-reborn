#include <Radiant/Core/Input.hpp>
#include <Radiant/Core/Application.hpp>

#include <GLFW/glfw3.h>

namespace Radiant
{
	bool Input::IsKeyPressed(KeyCode key)
	{
		const GLFWwindow* window = static_cast <const GLFWwindow*>(Application::GetInstance().GetWindow()->GetNativeWindow());
		auto state = glfwGetKey((GLFWwindow*)window, static_cast<int32_t>(key));

		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	bool Input::IsMouseButtonPressed(MouseButton button)
	{
		auto window = static_cast<const GLFWwindow*>(Application::GetInstance().GetWindow()->GetNativeWindow());
		auto state = glfwGetMouseButton((GLFWwindow*)window, static_cast<int32_t>(button));

		return state == GLFW_PRESS;
	}

	float Input::GetMouseX()
	{
		auto [x, y] = GetMousePosition();
		return (float)x;
	}

	float Input::GetMouseY()
	{
		auto [x, y] = GetMousePosition();
		return (float)y;
	}

	std::pair<float, float> Input::GetMousePosition()
	{
		auto window = static_cast<const GLFWwindow*>(Application::GetInstance().GetWindow()->GetNativeWindow());
		double x, y;
		glfwGetCursorPos((GLFWwindow*)window, &x, &y);
		return { (float)x, (float)y };
	}
}