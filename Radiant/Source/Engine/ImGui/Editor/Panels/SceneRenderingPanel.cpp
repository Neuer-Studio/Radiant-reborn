#include <Radiant/ImGui/Editor/Panels/SceneRenderingPanel.hpp>
#include <Radiant/Rendering/RenderingAPI.hpp>
#include <Radiant/Scene/Scene.hpp>
#include <Radiant/Scene/SceneRendering.hpp>

#include <imgui/imgui.h>

namespace Radiant
{

	SceneRenderingPanel::SceneRenderingPanel(const Memory::Shared<Scene>& scene /*= nullptr*/)
		: m_Context(scene)
	{

	}

	void SceneRenderingPanel::DrawImGuiUI()
	{
		ImGui::Begin("Scene Rendering");
		{
			auto width = m_Context->m_ViewportWidth;
			auto height = m_Context->m_ViewportHeight;

			const auto info = RenderingAPI::GetGraphicsInfo();

			std::string viewport = "Viewport: " + std::to_string(width) + " : " + std::to_string(height);
			std::string graphicInfo = "GPU: " + info.Renderer + "\nVersion: " + info.Version;

			ImGui::Text(viewport.c_str());
			ImGui::Text(graphicInfo.c_str());

			ImGui::Separator();
			ImGui::Text("Shader Parameters");
			static bool enable = false;
			static float rotation = 0.0f;
			ImGui::Checkbox("Radiance Prefiltering", &enable);
			Scene::GetSceneRendering()->SetRadiancePrefilter(enable);

			ImGui::SliderFloat("Env Map Rotation", &rotation, -360.0f, 360.0f);
			Scene::GetSceneRendering()->SetEnvMapRotation(rotation);

			ImGui::Separator();
		}
		ImGui::End();
	}

}