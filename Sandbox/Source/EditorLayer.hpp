#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <Radiant/Rendering/Scene/Entity.hpp>
#include <Radiant/Rendering/Scene/SceneRendering.hpp>
#include <ImGUI/imgui.h>

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
		}
		virtual void OnDetach()
		{

		}
		virtual void OnUpdate() override
		{
			m_Scene->UpdateScene(&CAM);

		}

		virtual void OnImGuiRender() override
		{
			// ImGui + Dockspace Setup ------------------------------------------------------------------------------
			ImGuiIO& io = ImGui::GetIO();
			ImGuiStyle& style = ImGui::GetStyle();
			auto boldFont = io.Fonts->Fonts[0];

			if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || (ImGui::IsMouseClicked(ImGuiMouseButton_Right)))
			{
			}

			io.ConfigWindowsResizeFromEdges = io.BackendFlags & ImGuiBackendFlags_HasMouseCursors;

			// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
			// because it would be confusing to have two docking targets within each others.
			ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_MenuBar;

			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->Pos);
			ImGui::SetNextWindowSize(viewport->Size);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

			bool isMaximized = Application::GetInstance().GetWindow()->IsWindowMaximized();

			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, isMaximized ? ImVec2(6.0f, 6.0f) : ImVec2(1.0f, 1.0f));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 3.0f);
			ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImVec4{ 0.0f, 0.0f, 0.0f, 0.0f });
			ImGui::Begin("DockSpace Demo", nullptr, window_flags);
			ImGui::PopStyleColor(); // MenuBarBg
			ImGui::PopStyleVar(2);

			ImGui::PopStyleVar(2);

			//m_Outliner->DrawComponentsUI();

			// Dockspace
			float minWinSizeX = style.WindowMinSize.x;
			style.WindowMinSize.x = 370.0f;
			ImGui::DockSpace(ImGui::GetID("MyDockspace"));
			style.WindowMinSize.x = minWinSizeX;

			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
			ImGui::Begin("Viewport");
			{
				auto viewportOffset = ImGui::GetCursorPos(); // includes tab bar
				auto viewportSize = ImGui::GetContentRegionAvail();

				CAM.SetProjectionMatrix(glm::perspectiveFov(glm::radians(45.0f), viewportSize.x, viewportSize.y, 0.1f, 10000.0f));
				CAM.SetViewportSize((uint32_t)viewportSize.x, (uint32_t)viewportSize.y);

				Scene::GetSceneRendering()->SetSceneVeiwPortSize({viewportSize.x, viewportSize.y});

				if(Scene::GetSceneRendering()->GetFinalPassImage())
					ImGui::Image((void*)Scene::GetSceneRendering()->GetFinalPassImage()->GetTextureID(), viewportSize, { 0, 1 }, { 1, 0 });


				static int counter = 0;
				auto windowSize = ImGui::GetWindowSize();
				ImVec2 minBound = ImGui::GetWindowPos();
				minBound.x += viewportOffset.x;
				minBound.y += viewportOffset.y;

			}

			ImGui::End();
			ImGui::PopStyleVar();

			//m_Outliner->DrawImGuiUI();
			//m_ScenePanel->DrawImGuiUI();
			ImGui::End();

		}
	private:
		Memory::Shared<Scene> m_Scene;
		Camera CAM;

	};
}