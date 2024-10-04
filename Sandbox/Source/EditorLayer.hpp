#pragma once

#include <Radiant/Radiant.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <Radiant/Scene/Entity.hpp>
#include <Radiant/Rendering/SceneRendering.hpp>
#include <Radiant/ImGui/Editor/Panels/SceneHierarchyPanel.hpp>
#include <Radiant/ImGui/Editor/Panels/SceneRenderingPanel.hpp>
#include <ImGUI/imgui.h>

namespace Radiant
{
    class EditorLayer : public Common::Layer
    {
    public:
        EditorLayer() : Common::Layer( "EditorLayer" ), m_EditorCamera( 1920, 1080 )
        {
        }

        virtual void OnAttach();
        virtual void OnDetach()
        {
        }
        virtual void OnUpdate( Common::Timestep ts ) override;

        virtual void OnEvent( Common::Event& e ) override;

        virtual void OnImGuiRender() override;

    private:
        bool OnMouseButtonPressed( Common::MouseButtonPressedEvent& e );

    private:
        void LoadScene();

    private:
        Common::Memory::Shared<Scene> m_Scene;
        ImVec2                        m_ViewportSize;
        Camera                        m_EditorCamera;
        Entity*                       m_SelectedEntity = nullptr; // TEMP
        glm::vec2                     m_ViewportBounds[2];

        std::pair<uint32_t, uint32_t> m_MouseClick;

        Common::Memory::Shared<SceneHierarchyPanel> m_SceneHierarchyPanel;
        Common::Memory::Shared<SceneRenderingPanel> m_SceneRenderingPanel;
    };

} // namespace Radiant