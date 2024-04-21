#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <Radiant/Scene/Entity.hpp>
#include <Radiant/Scene/SceneRendering.hpp>
#include <Radiant/ImGui/Editor/Panels/PanelOutliner.hpp>
#include <Radiant/ImGui/Editor/Panels/SceneRenderingPanel.hpp>
#include <ImGUI/imgui.h>

namespace Radiant
{
	class EditorLayer : public Layer
	{
	public:
		EditorLayer()
			: Layer("EditorLayer"), m_EditorCamera(glm::perspectiveFov(glm::radians(45.0f), 1280.0f, 720.0f, 0.1f, 1000.0f))
		{}

		virtual void OnAttach()
		{
			m_Scene = Memory::Shared<Scene>::Create("Test Scene");

			auto env = m_Scene->CreateEnvironmentScene("Resources/Textures/HDR/environment.hdr");
			m_Scene->SetEnvironment(env);
			m_Outliner = new PanelOutliner(m_Scene);
			m_SceneRenderingPanel = new SceneRenderingPanel(m_Scene);
			
		}
		virtual void OnDetach()
		{

		}
		virtual void OnUpdate(Timestep ts) override
		{
			m_EditorCamera.OnUpdate(ts);
			m_Scene->OnUpdate(ts, m_EditorCamera);
		}

		virtual void OnEvent(Radiant::Event& e) override
		{
			m_EditorCamera.OnEvent(e);

			EventManager eventManager(e);
			eventManager.Notify<EventWindowResize>([this](const EventWindowResize& e) -> bool
				{
					return false;
				});
		}

		virtual void OnImGuiRender() override
		{
			static bool p_open = true;

			static bool opt_fullscreen_persistant = true;
			static ImGuiDockNodeFlags opt_flags = ImGuiDockNodeFlags_None;
			bool opt_fullscreen = opt_fullscreen_persistant;

			// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
			ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
			if (opt_fullscreen)
			{
				ImGuiViewport* viewport = ImGui::GetMainViewport();
				ImGui::SetNextWindowPos(viewport->Pos);
				ImGui::SetNextWindowSize(viewport->Size);
				ImGui::SetNextWindowViewport(viewport->ID);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
				window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
				window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
			}

			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
			ImGui::Begin("DockSpace Demo", &p_open, window_flags);
			ImGui::PopStyleVar();

			if (opt_fullscreen)
				ImGui::PopStyleVar(2);

			// ImGui + Dockspace Setup ------------------------------------------------------------------------------
			ImGuiIO& io = ImGui::GetIO();
			ImGuiStyle& style = ImGui::GetStyle();
			auto boldFont = io.Fonts->Fonts[0];

			bool isMaximized = Application::GetInstance().GetWindow()->IsWindowMaximized();

			m_Outliner->DrawComponentsUI();

			// Dockspace
			float minWinSizeX = style.WindowMinSize.x;
			style.WindowMinSize.x = 370.0f;
			ImGui::DockSpace(ImGui::GetID("MyDockspace"));
			style.WindowMinSize.x = minWinSizeX;

			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
			ImGui::Begin("Viewport");
			{
				auto viewportOffset = ImGui::GetCursorPos(); // includes tab bar
				m_ViewportSize = ImGui::GetContentRegionAvail();

				Scene::GetSceneRendering()->SetSceneVeiwPortSize({ m_ViewportSize.x, m_ViewportSize.y });

				m_EditorCamera.SetProjectionMatrix(glm::perspectiveFov(glm::radians(45.0f), m_ViewportSize.x, m_ViewportSize.y, 0.1f, 1000.0f));
				m_EditorCamera.SetViewportSize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);

				if(Scene::GetSceneRendering()->GetFinalPassImage())
					ImGui::Image((void*)Scene::GetSceneRendering()->GetFinalPassImage()->GetTextureID(), m_ViewportSize, { 0, 1 }, { 1, 0 });


				static int counter = 0;
				auto windowSize = ImGui::GetWindowSize();
				ImVec2 minBound = ImGui::GetWindowPos();
				minBound.x += viewportOffset.x;
				minBound.y += viewportOffset.y;

			}

			Scene::GetSceneRendering()->OnImGuiRender();

			ImGui::End();
			ImGui::PopStyleVar();

			m_Outliner->DrawImGuiUI();
			m_SceneRenderingPanel->DrawImGuiUI();
			//m_ScenePanel->DrawImGuiUI();
			ImGui::Begin("Models");
			if (ImGui::TreeNode("Shaders"))
			{
				auto& shaders =Shader::s_AllShaders;
				for (auto& shader : shaders)
				{
					if (ImGui::TreeNode(shader->GetShaderName().c_str()))
					{
						std::string buttonName = "Reload##" + shader->GetShaderName();
						if (ImGui::Button(buttonName.c_str()))
							shader->Reload();
						ImGui::TreePop();
					}
				}
				ImGui::TreePop();
			}

			ImGui::End();
			ImGui::End();
		}


	private:
		Memory::Shared<Scene> m_Scene;
		ImVec2 m_ViewportSize;
		Camera m_EditorCamera;

		Memory::Shared<PanelOutliner> m_Outliner;
		Memory::Shared<SceneRenderingPanel> m_SceneRenderingPanel;
	};
}