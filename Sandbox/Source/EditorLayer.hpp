#pragma once

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

        virtual void OnAttach()
        {
            m_Scene = Memory::Shared<Scene>::Create( "Test Scene" );

            /*auto env = m_Scene->CreateEnvironmentScene("Resources/Textures/HDR/environment.hdr");
            m_Scene->SetEnvironment(env);*/
            m_SceneHierarchyPanel = new SceneHierarchyPanel( m_Scene );
            m_SceneRenderingPanel = new SceneRenderingPanel( m_Scene );
        }
        virtual void OnDetach()
        {
        }
        virtual void OnUpdate( Timestep ts ) override
        {
            m_EditorCamera.OnUpdate( ts );

            SceneUpdateInformation info;
            info.Camera   = m_EditorCamera;
            info.TimeStep = ts;
            info.Width    = m_ViewportSize.x;
            info.Height   = m_ViewportSize.y;

            m_Scene->OnUpdate( info );
        }

        virtual void OnEvent( Radiant::Event& e ) override
        {
            m_EditorCamera.OnEvent( e );

            EventManager eventManager( e );
            eventManager.Notify<EventWindowResize>( [this]( const EventWindowResize& e ) -> bool
                                                    { return false; } );

            eventManager.Notify<MouseButtonPressedEvent>( [this]( MouseButtonPressedEvent& e ) -> bool
                                                          { return this->OnMouseButtonPressed( e ); } );
        }

        virtual void OnImGuiRender() override
        {
            static bool p_open = true;

            static bool               opt_fullscreen_persistant = true;
            static ImGuiDockNodeFlags opt_flags                 = ImGuiDockNodeFlags_None;
            bool                      opt_fullscreen            = opt_fullscreen_persistant;

            // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
            // because it would be confusing to have two docking targets within each others.
            ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
            if ( opt_fullscreen )
            {
                ImGuiViewport* viewport = ImGui::GetMainViewport();
                ImGui::SetNextWindowPos( viewport->Pos );
                ImGui::SetNextWindowSize( viewport->Size );
                ImGui::SetNextWindowViewport( viewport->ID );
                ImGui::PushStyleVar( ImGuiStyleVar_WindowRounding, 0.0f );
                ImGui::PushStyleVar( ImGuiStyleVar_WindowBorderSize, 0.0f );
                window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                                ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
                window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
            }

            ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 0.0f, 0.0f ) );
            ImGui::Begin( "DockSpace Demo", &p_open, window_flags );
            ImGui::PopStyleVar();

            if ( opt_fullscreen )
                ImGui::PopStyleVar( 2 );

            // ImGui + Dockspace Setup
            // ------------------------------------------------------------------------------
            ImGuiIO&    io       = ImGui::GetIO();
            ImGuiStyle& style    = ImGui::GetStyle();
            auto        boldFont = io.Fonts->Fonts[0];

            bool isMaximized = Application::GetInstance().GetWindow()->IsWindowMaximized();

            m_SceneHierarchyPanel->DrawComponentsUI();

            // Dockspace
            float minWinSizeX     = style.WindowMinSize.x;
            style.WindowMinSize.x = 370.0f;
            ImGui::DockSpace( ImGui::GetID( "MyDockspace" ) );
            style.WindowMinSize.x = minWinSizeX;

            ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 0, 0 ) );
            ImGui::Begin( "Viewport" );
            {
                auto viewportOffset = ImGui::GetCursorPos(); // includes tab bar
                m_ViewportSize      = ImGui::GetContentRegionAvail();

                m_EditorCamera.SetViewportSize( (uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y );

                if ( m_Scene->GetFinalPassImage() ) // TODO: move to scene
                    ImGui::Image( (void*)m_Scene->GetFinalPassImage()->GetTextureID(), m_ViewportSize, { 0, 1 },
                                  { 1, 0 } );

                static int counter    = 0;
                auto       windowSize = ImGui::GetWindowSize();
                ImVec2     minBound   = ImGui::GetWindowPos();
                minBound.x += viewportOffset.x;
                minBound.y += viewportOffset.y;

                ImVec2 maxBound     = { minBound.x + windowSize.x, minBound.y + windowSize.y };
                m_ViewportBounds[0] = { minBound.x, minBound.y };
                m_ViewportBounds[1] = { maxBound.x, maxBound.y };
            }

            // SceneRendering::OnImGuiRender();

            ImGui::End();
            ImGui::PopStyleVar();

            m_SceneHierarchyPanel->DrawImGuiUI();
            m_SceneRenderingPanel->DrawImGuiUI();
            // m_ScenePanel->DrawImGuiUI();
            ImGui::Begin( "Models" );
            if ( ImGui::TreeNode( "Shaders" ) )
            {
                auto& shaders = Shader::s_AllShaders;
                for ( auto& shader : shaders )
                {
                    if ( ImGui::TreeNode( shader->GetShaderName().c_str() ) )
                    {
                        std::string buttonName = "Reload##" + shader->GetShaderName();
                        if ( ImGui::Button( buttonName.c_str() ) )
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
        bool EditorLayer::OnMouseButtonPressed( MouseButtonPressedEvent& e );

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

    bool EditorLayer::OnMouseButtonPressed( MouseButtonPressedEvent& e )
    {
        if (e.GetMouseButton() != MouseButton::Left || Input::Keyboard::IsKeyPressed(KeyCode::LeftControl))
        {
            return false;
        }
        // https://antongerdelan.net/opengl/raycasting.html
        auto [mx, my] = ImGui::GetMousePos();

        mx -= m_ViewportBounds[0].x;
        my -= m_ViewportBounds[0].y;
        auto viewportWidth  = m_ViewportBounds[1].x - m_ViewportBounds[0].x;
        auto viewportHeight = m_ViewportBounds[1].y - m_ViewportBounds[0].y;

        mx = ( mx / viewportWidth ) * 2.0f - 1.0f;
        my = ( ( my / viewportHeight ) * 2.0f - 1.0f ) * -1.0f;

        const auto [origin, direction] = Math::Ray::CastRay( m_EditorCamera, mx, my );

        auto meshEntities = m_Scene->GetAllEntitiesWith<MeshComponent>();
        for ( auto e : meshEntities )
        {
            Entity entity = { e, m_Scene.Raw() };
            auto   mesh   = entity.GetComponent<MeshComponent>().Mesh;
            if ( !mesh )
                continue;
            auto& submeshes = mesh->GetSubmeshes();
            for ( uint32_t i = 0; i < submeshes.size(); i++ )
            {
                auto& submesh = submeshes[i];
                Math::Ray   ray = { glm::inverse( entity.GetTransform() * submesh.Transform ) * glm::vec4( origin, 1.0f ),
                              glm::inverse( glm::mat3( entity.GetTransform() ) * glm::mat3( submesh.Transform ) ) *
                                   direction };

                bool intersects = ray.IntersectsAABB( submesh.BoundingBox );
                if ( intersects )
                {
                    RA_WARN( "INTERSECTION: {0}", submesh.NodeName );
                }
            }
        }

        return false;
    }

} // namespace Radiant