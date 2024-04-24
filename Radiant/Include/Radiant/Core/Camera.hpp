#pragma once

#include <Radiant/Core/Timestep.hpp>
#include <Radiant/Core/Events/MouseEvents.hpp>

#include <glm/glm.hpp>

namespace Radiant
{
	class Camera
	{
	public:
		Camera(uint32_t width, uint32_t height);
		Camera();

		void SetViewportSize(uint32_t width, uint32_t height);
		const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
		glm::mat4 GetViewProjection() const { return m_ProjectionMatrix * m_ViewMatrix; }
		const glm::vec3 GetPosition() const { return m_FocalPoint - GetForwardDirection() * m_Distance; }

		void OnUpdate(Timestep ts);
		void SetActive(bool Active) { m_IsActive = Active; }

		const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }

		void OnEvent(Event& e);
	private:
		void SetProjectionMatrix(const glm::mat4& projectionMatrix) { m_ProjectionMatrix = projectionMatrix; }
		glm::quat GetOrientation() const;
		glm::vec3 GetUpDirection() const;
		glm::vec3 GetRightDirection() const;
		glm::vec3 GetForwardDirection() const;

		void UpdateCameraView();


	private:
		glm::mat4 m_ProjectionMatrix = glm::mat4(1.0f);
		glm::mat4 m_ViewMatrix = glm::mat4(1.0f);
		glm::vec3 m_Location = glm::vec3(0.0f);
		glm::vec3 m_LocationDelta = glm::vec3(0.0f);
		glm::vec3 m_Direction;
		glm::vec3 m_FocalPoint = glm::vec3(0.0f);

		glm::vec3 m_RightDirection;

		float m_Distance = 0.0f;
		glm::vec2 m_InitialMousePosition = glm::vec2(0.0f);

		float m_Pitch = 0.0f, m_PitchDelta = 0.0f;
		float m_Yaw = 0.0f, m_YawDelta = 0.0f;

		glm::vec3 m_Rotation = glm::vec3(0.0f);

		float m_VerticalFOV = 45.0f;
		float m_NearPlane = 0.1f;
		float m_FarPlane = 1000.0f;

		bool m_IsActive = false;
	};

}