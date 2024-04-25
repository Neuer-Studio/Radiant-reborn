#pragma once

#include "KeyCodes.hpp"
#include "MouseButton.hpp"

namespace Radiant::Input
{
	enum class MouseState
	{
		Visible, Hidden, Locked
	};

	class Mouse
	{
	public:
		static Mouse& Get();

		bool IsMouseButtonPressed(MouseButton button);
		float GetMouseX();
		float GetMouseY();
		std::pair<float, float> GetMousePosition();

		void SetVisibility(MouseState state) { m_MouseState = state; }

		const MouseState GetVisibility() const { return m_MouseState; }
	private:
		MouseState m_MouseState = MouseState::Visible;
	};

	class Keyboard
	{
	public:
		static bool IsKeyPressed(KeyCode keycode);
	};
}