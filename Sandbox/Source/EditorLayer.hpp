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
    class EditorLayer : public Layer
    {
    public:
        EditorLayer() : Layer( "EditorLayer" ), m_EditorCamera( 1920, 1080 )
        {
        }

        virtual void OnAttach();
        virtual void OnDetach()
        {
        }
        virtual void OnUpdate( Timestep ts ) override;

        virtual void OnEvent( Radiant::Event& e ) override;

        virtual void OnImGuiRender() override;

    private:
        bool OnMouseButtonPressed( MouseButtonPressedEvent& e );
    private:
        void LoadScene();
    private:
        Memory::Shared<Scene> m_Scene;
        ImVec2                m_ViewportSize;
        Camera                m_EditorCamera;
        Entity*               m_SelectedEntity = nullptr; // TEMP
        glm::vec2             m_ViewportBounds[2];

        std::pair<uint32_t, uint32_t> m_MouseClick;

        Memory::Shared<SceneHierarchyPanel> m_SceneHierarchyPanel;
        Memory::Shared<SceneRenderingPanel> m_SceneRenderingPanel;
    };

} // namespace Radiant