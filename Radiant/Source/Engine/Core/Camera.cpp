#include <Radiant/Core/Camera.hpp>
#include <Radiant/Core/Application.hpp>
#include <Radiant/Core/Input.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#define M_PI 3.14159f

namespace Radiant
{
	Camera::Camera()
	{
		SetViewportSize(1, 1);

		constexpr glm::vec3 InitialPosition = { 5, 5, 5 };
		m_Distance = glm::distance(InitialPosition, m_FocalPoint);

		m_Yaw = 3.0f * glm::pi<float>() / 4.0f;
		m_Pitch = glm::pi<float>() / 4.0f;

		m_Location = m_FocalPoint - GetForwardDirection() * m_Distance + m_LocationDelta;
		const glm::quat Orientation = GetOrientation();
		m_Direction = glm::eulerAngles(Orientation) * (180.0f / glm::pi<float>());
		m_ViewMatrix = glm::translate(glm::mat4(1.0f), m_Location) * glm::toMat4(Orientation);
		m_ViewMatrix = glm::inverse(m_ViewMatrix);

	}

	Camera::Camera(uint32_t width, uint32_t height)
	{
		SetViewportSize(width, height);

		constexpr glm::vec3 InitialPosition = { 5, 5, 5 };
		m_Distance = glm::distance(InitialPosition, m_FocalPoint);

		m_Yaw = 3.0f * glm::pi<float>() / 4.0f;
		m_Pitch = glm::pi<float>() / 4.0f;

		m_Location = m_FocalPoint - GetForwardDirection() * m_Distance + m_LocationDelta;
		const glm::quat Orientation = GetOrientation();
		m_Direction = glm::eulerAngles(Orientation) * (180.0f / glm::pi<float>());
		m_ViewMatrix = glm::translate(glm::mat4(1.0f), m_Location) * glm::toMat4(Orientation);
		m_ViewMatrix = glm::inverse(m_ViewMatrix);
	}

	void Camera::SetViewportSize(uint32_t width, uint32_t height)
	{
		SetProjectionMatrix(glm::perspectiveFov(glm::radians(m_VerticalFOV), float(width), float(height), m_NearPlane, m_FarPlane));
	}

	void Camera::OnUpdate(Timestep ts)
	{
		const glm::vec2& MousePosition{ Input::Mouse::Get().GetMouseX(), Input::Mouse::Get().GetMouseY() };
		const glm::vec2 MouseDelta = (MousePosition - m_InitialMousePosition) * 0.002f;

		if (Input::Mouse::Get().IsMouseButtonPressed(MouseButton::Right))
		{
			Input::Mouse::Get().SetCursorMode(Input::MouseState::Locked);

			const float YawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;
			const float CameraSpeed = 0.01f * ts.GetMilliseconds();
			const float RotationSpeed = 0.3f * ts.GetMilliseconds();

			if (Input::Keyboard::IsKeyPressed(KeyCode::Q))
				m_LocationDelta -= CameraSpeed * glm::vec3{ 0.f, YawSign, 0.f };
			if (Input::Keyboard::IsKeyPressed(KeyCode::E))
				m_LocationDelta += CameraSpeed * glm::vec3{ 0.f, YawSign, 0.f };
			if (Input::Keyboard::IsKeyPressed(KeyCode::S))
				m_LocationDelta -= CameraSpeed * m_Direction;
			if (Input::Keyboard::IsKeyPressed(KeyCode::W))
				m_LocationDelta += CameraSpeed * m_Direction;
			if (Input::Keyboard::IsKeyPressed(KeyCode::A))
				m_LocationDelta -= CameraSpeed * m_RightDirection;
			if (Input::Keyboard::IsKeyPressed(KeyCode::D))
				m_LocationDelta += CameraSpeed * m_RightDirection;

			constexpr float MaxRate = 0.12f;
			m_YawDelta += glm::clamp(YawSign * MouseDelta.x * RotationSpeed, -MaxRate, MaxRate);
			m_PitchDelta += glm::clamp(MouseDelta.y * RotationSpeed, -MaxRate, MaxRate);

			m_RightDirection = glm::cross(m_Direction, glm::vec3{ 0.f, YawSign, 0.f });

			m_Direction = glm::rotate(glm::normalize(glm::cross(glm::angleAxis(-m_PitchDelta, m_RightDirection),
				glm::angleAxis(-m_YawDelta, glm::vec3{ 0.f, YawSign, 0.f }))), m_Direction);

			const float distance = glm::distance(m_FocalPoint, m_Location);
			m_FocalPoint = m_Location + GetForwardDirection() * distance;
			m_Distance = distance;
		}
		else
		{
			Input::Mouse::Get().SetCursorMode(Input::MouseState::Visible);
		}

		m_InitialMousePosition = MousePosition;
		m_Location += m_LocationDelta;
		m_Yaw += m_YawDelta;
		m_Pitch += m_PitchDelta;

		static constexpr float MaxPitch = glm::radians(89.0f);
		static constexpr float MinPitch = -MaxPitch;
		m_Pitch = glm::clamp(m_Pitch, MinPitch, MaxPitch);

		UpdateCameraView();
	}

	void Camera::UpdateCameraView()
	{
		const float YawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;

		// Extra step to handle the problem when the camera direction is the same as the up vector
		const float CosAngle = glm::dot(GetForwardDirection(), GetUpDirection());
		if (CosAngle * YawSign > 0.99f)
			m_PitchDelta = 0.f;

		const glm::vec3 LookDirection = m_Location + GetForwardDirection();
		m_Direction = glm::normalize(LookDirection - m_Location);
		m_Distance = glm::distance(m_Location, m_FocalPoint);
		m_ViewMatrix = glm::lookAt(m_Location, LookDirection, glm::vec3{ 0.f, YawSign, 0.f });

		// Damping
		m_YawDelta *= 0.6f;
		m_PitchDelta *= 0.6f;
		m_LocationDelta *= 0.8f;
	}

	glm::quat Camera::GetOrientation() const { return glm::quat(glm::vec3(-m_Pitch - m_PitchDelta, -m_Yaw - m_YawDelta, 0.0f)); }
	glm::vec3 Camera::GetUpDirection() const { return glm::rotate(GetOrientation(), glm::vec3(0.0f, 1.0f, 0.0f)); }
	glm::vec3 Camera::GetRightDirection() const { return glm::rotate(GetOrientation(), glm::vec3(1.0f, 0.0f, 0.0f)); }
	glm::vec3 Camera::GetForwardDirection() const { return glm::rotate(GetOrientation(), glm::vec3(0.0f, 0.0f, -1.0f)); }

	void Camera::OnEvent(Event& e)
	{
		EventManager eventManager(e);
		eventManager.Notify<EventWindowResize>([this](const EventWindowResize& event)
			{
				//this->SetViewportSize(event.width, event.height);
				return false;
			});
	}
}