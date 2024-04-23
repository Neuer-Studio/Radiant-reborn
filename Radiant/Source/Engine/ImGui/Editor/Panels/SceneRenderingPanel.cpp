#include <Radiant/ImGui/Editor/Panels/SceneRenderingPanel.hpp>
#include <Radiant/Rendering/RendererAPI.hpp>
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
			auto width = m_Context->GetSceneUpdateInfo().Width;
			auto height = m_Context->GetSceneUpdateInfo().Height;

			const auto info = RendererAPI::GetGraphicsInfo();

			std::string viewport = "Viewport: " + std::to_string(width) + " : " + std::to_string(height);
			std::string graphicInfo = "GPU: " + info.Renderer + "\nVersion: " + info.Version;

			ImGui::Text(viewport.c_str());
			ImGui::Text(graphicInfo.c_str());

			ImGui::Separator();
			ImGui::Text("Shader Parameters");
			static float valueIBLContribution = 1.0;
			static float rotation = 0.0f;
			ImGui::SliderFloat("Radiance Prefiltering", &valueIBLContribution, 0.0, 1.0);
			SceneRendering::SetIBLContribution(valueIBLContribution);

			int samples = (int)m_Context->m_SamplesCount;
			ImGui::SliderFloat("Env Map Rotation", &rotation, -360.0f, 360.0f);
			ImGui::SliderInt("Samples Scene", &samples, 1, 16);
			SceneRendering::SetEnvMapRotation(rotation);

			ImGui::Separator();

			m_Context->m_SamplesCount = (uint32_t)samples;
		}
		ImGui::End();
	}

}