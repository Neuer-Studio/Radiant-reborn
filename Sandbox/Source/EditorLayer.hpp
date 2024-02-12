#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <Radiant/Rendering/Scene/Entity.hpp>
#include <Radiant/Rendering/Scene/SceneRendering.hpp>

namespace Radiant
{
	class EditorLayer : public Layer
	{
	public:
		EditorLayer()
			: Layer("EditorLayer"), CAM(glm::perspectiveFov(glm::radians(45.0f), 1280.0f, 720.0f, 0.1f, 10000.0f))
		{}

		virtual void OnAttach()
		{
			m_Scene = Memory::Shared<Scene>::Create("Test Scene");

			auto env = m_Scene->CreateEnvironmentScene("Resources/Textures/HDR/environment.hdr");
			m_Scene->SetEnvironment(env);

			CAM.SetProjectionMatrix(glm::perspectiveFov(glm::radians(45.0f), 1280.0f, 720.0f, 0.1f, 10000.0f));
			CAM.SetViewportSize((uint32_t)1280.0f, (uint32_t)720.0f);
		}
		virtual void OnDetach()
		{

		}
		virtual void OnUpdate() override
		{
			m_Scene->UpdateScene(&CAM);

		}
	private:
		Memory::Shared<Scene> m_Scene;
		Camera CAM;

	};
}