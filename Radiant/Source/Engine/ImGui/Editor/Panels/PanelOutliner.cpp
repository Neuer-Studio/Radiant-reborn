#include <Radiant/ImGui/Editor/Panels/PanelOutliner.hpp>
#include <Radiant/Scene/Entity.hpp>
#include <Radiant/Scene/Component.hpp>
#include <Radiant/ImGui/Utilities/UI.hpp>
#include <Radiant/Scene/SceneRendering.hpp>
#include <Radiant/Scene/Entity.hpp>

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Radiant
{
	namespace
	{
		template <typename T, typename UIFunction>
		static void DrawComponentUI(const std::string& name, Entity entity, UIFunction uiFunction)
		{
			const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
			if (entity.HasComponent<T>())
			{
				ImGui::PushID(name.c_str());
				auto& component = entity.GetComponent<T>();
				ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
				float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
				ImGui::Separator();
				bool open = ImGui::TreeNodeEx("##dummy_id", treeNodeFlags, name.c_str());
				ImGui::PopStyleVar();
				ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
				if (ImGui::Button("+", ImVec2{ lineHeight, lineHeight }))
				{
					ImGui::OpenPopup("ComponentSettings");
				}

				bool removeComponent = false;
				if (ImGui::BeginPopup("ComponentSettings"))
				{
					if (ImGui::MenuItem("Remove component"))
						removeComponent = true;

					ImGui::EndPopup();
				}

				if (open)
				{
					uiFunction(component);
					ImGui::TreePop();
				}

				if (removeComponent)
					entity.RemoveComponent<T>();

				::ImGui::PopID();
			}
		}

		static bool DrawVec3UI(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f)
		{
			bool modified = false;

			ImGuiIO& io = ImGui::GetIO();
			auto boldFont = io.Fonts->Fonts[0];

			ImGui::PushID(label.c_str());

			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, columnWidth);
			ImGui::Text(label.c_str());
			ImGui::NextColumn();

			ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.3f, 0.8f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.4f, 0.9f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.3f, 0.8f, 1.0f });
			ImGui::PushFont(boldFont);
			if (ImGui::Button("X", buttonSize))
			{
				values.x = resetValue;
				modified = true;
			}

			ImGui::PopFont();
			ImGui::PopStyleColor(3);

			ImGui::SameLine();
			modified |= ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
			ImGui::PopItemWidth();
			ImGui::SameLine();

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
			ImGui::PushFont(boldFont);
			if (ImGui::Button("Y", buttonSize))
			{
				values.y = resetValue;
				modified = true;
			}

			ImGui::PopFont();
			ImGui::PopStyleColor(3);

			ImGui::SameLine();
			modified |= ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
			ImGui::PopItemWidth();
			ImGui::SameLine();

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
			ImGui::PushFont(boldFont);
			if (ImGui::Button("Z", buttonSize))
			{
				values.z = resetValue;
				modified = true;
			}

			ImGui::PopFont();
			ImGui::PopStyleColor(3);

			ImGui::SameLine();
			modified |= ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
			ImGui::PopItemWidth();

			ImGui::PopStyleVar();

			ImGui::Columns(1);

			ImGui::PopID();

			return modified;
		}

	}

	PanelOutliner::PanelOutliner(const Memory::Shared<Scene>& context)
		: m_Context(context)
	{
	}

	void PanelOutliner::SetContext(const Memory::Shared<Scene>& scene)
	{
		m_Context = scene;
	}

	void PanelOutliner::DrawImGuiUI()
	{
		ImGui::Begin("Outliner");
		if (m_Context)
		{
			m_Context->m_Registry.each([&](entt::entity entity)
				{
					Entity e(entity, m_Context.Ptr());
					if(e.HasComponent<IDComponent>())
						DrawEntityNodeUI(e);
				});
		}

		ImGui::End();

		ImGui::Begin("Properties");

		if (m_SelectionContext)
			DrawPropertiesUI(m_SelectionContext);

		ImGui::End();
	}

	void PanelOutliner::DrawComponentsUI(const std::string& ButtonName, float x, float y)
	{
		if (m_Context)
		{
			ImGui::Indent(20);
			if (ImGui::Button(ButtonName.c_str(), ImVec2(x, y)))
			{
				ImGui::OpenPopup("AddEntityMenu");
			}
			ImGui::Dummy(ImVec2(0.0f, 20.0f));
			ImGui::Unindent(20);

			if (ImGui::BeginPopup("AddEntityMenu"))
			{
				ImGui::Indent(20.0f);

				if (ImGui::MenuItem("Empty Entity"))
				{
					m_Context->CreateEntity("Empty Entity");
				}

				ImGui::Spacing();

				/*if (ImGui::MenuItem("Camera"))
				{
					Entity* entity = m_Context->CreateEntity("Camera");
					auto& camera = CreateNewComponent<CameraComponent>();
					camera->Camera.SetProjectionMatrix(glm::perspectiveFov(glm::radians(45.0f), 1280.0f, 720.0f, 0.1f, 10000.0f));
					entity->AddComponent(camera);

				}

				ImGui::Spacing();*/

				if (ImGui::MenuItem("Mesh"))
				{
					auto entity = m_Context->CreateEntity("Mesh");
					entity.AddComponent<MeshComponent>();
				}

				if (ImGui::MenuItem("Camera"))
				{
					auto entity = m_Context->CreateEntity("Camera");
					entity.AddComponent<CameraComponent>().Camera.SetProjectionMatrix(glm::perspectiveFov(glm::radians(45.0f), 1280.0f, 720.0f, 0.1f, 10000.0f));
				}

				ImGui::Spacing();

				if (ImGui::BeginMenu("3D"))
				{
					if (ImGui::Button("Sphere Collider"))
					{
					}

					ImGui::EndMenu();
				}

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

			/*	if (ImGui::BeginMenu("Light"))
				{
					if (ImGui::MenuItem("Sky Light"))
					{
						Entity* entity = m_Context->CreateEntity("SkyLight");
						auto skybox = CreateNewComponent<SkyBoxComponent>();
						auto [radiance, irradiance] = SceneRendering::CreateEnvironmentMap("Resources/Envorement/HDR/birchwood_4k.hdr");
						skybox->Environment = Memory::Shared<Environment>::Create(radiance, irradiance);
						entity->AddComponent(skybox);
					}

					ImGui::Spacing();

					if (ImGui::MenuItem("Directional Light"))
					{
						Entity* entity = m_Context->CreateEntity("Directional Light");
						auto light = CreateNewComponent<DirectionLightComponent>();
						entity->AddComponent(light);
					}

					ImGui::Spacing();

					if (ImGui::MenuItem("Point Light"))
					{
					}

					ImGui::EndMenu();
				}*/

				ImGui::Unindent();
				ImGui::EndPopup();
			}
		}
	}

	void PanelOutliner::DrawEntityNodeUI(Entity entity)
	{
		const char* name = "Unnamed Entity";

		if (entity.HasComponent<TagComponent>())
			name = entity.GetComponent<TagComponent>().Tag.c_str();

		ImGuiTreeNodeFlags flags = (entity == m_SelectionContext ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
		bool opened = ImGui::TreeNodeEx((void*)&entity, flags, name);

		if (ImGui::IsItemClicked())
		{
			m_SelectionContext = entity;
		}

		if (opened)
		{
			// TODO: Children
			ImGui::TreePop();
		}
	}

	void PanelOutliner::DrawPropertiesUI(Entity entity)
	{
		ImGui::AlignTextToFramePadding();
		// ...
		ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

		if (m_SelectionContext.HasComponent<TagComponent>())
		{
			auto& tag = m_SelectionContext.GetComponent<TagComponent>().Tag;
			char buffer[256];
			memset(buffer, 0, 256);
			memcpy(buffer, tag.c_str(), tag.length());
			if (ImGui::InputText("##Tag", buffer, 256))
			{
				tag = std::string(buffer);
			}
		}

		std::string TextUUID("UUID: " + m_SelectionContext.GetComponent<IDComponent>().ID.ToString());

		ImGui::Text(TextUUID.c_str());
		/*::ImGui::Unindent(contentRegionAvailable.x * 0.05f);
		::ImGui::PopItemWidth();*/

		DrawComponentUI<MeshComponent>("Mesh", entity, [&](MeshComponent& mc)
			{
				auto& mesh = entity.GetComponent<MeshComponent>().Mesh;
				UI::BeginPropertyGrid();

				ImGui::Text(entity.GetComponent<TagComponent>().Tag.c_str());
				ImGui::NextColumn();
				ImGui::PushItemWidth(-1);

				ImVec2 originalButtonTextAlign = ImGui::GetStyle().ButtonTextAlign;
				ImGui::GetStyle().ButtonTextAlign = { 0.0f, 0.5f };
				float width = ImGui::GetContentRegionAvail().x - 0.0f;
				UI::PushID();

				float itemHeight = 28.0f;

				std::string buttonName;
				if (mesh)
					buttonName = mesh->GetName();
				else
					buttonName = "Null";

				if (ImGui::Button(buttonName.c_str(), { width, itemHeight }))
				{
					std::string file = Utils::FileSystem::OpenFileDialog().string();
					if (!file.empty())
					{
						mesh = Memory::Shared<Mesh>::Create(file);

					}
				}

				UI::PopID();
				ImGui::GetStyle().ButtonTextAlign = originalButtonTextAlign;
				ImGui::PopItemWidth();

				UI::EndPropertyGrid();

				UI::BeginPropertyGrid();
				UI::EndPropertyGrid();
			});
	}
}