#pragma once

#include <string>
#include <Common/Core/Events/WindowEvents.hpp>
#include <Common/Core/Events/MouseEvents.hpp>
#include <Common/Core/Timestep.hpp>

#include <Common/Core/Window.hpp>

#include <Radiant/Rendering/RendererAPI.hpp>
#include <Common/Core/LayerStack.hpp>
#include <Radiant/ImGui/ImGuiLayer.hpp>

namespace Radiant
{
    struct ApplicationSpecification
    {
        std::string      Name        = "TheRock";
        uint32_t         WindowWidth = 1600, WindowHeight = 900;
        bool             Fullscreen = false;
        RenderingAPIType APIType    = RenderingAPIType::OpenGL;
    };

    class Application
    {
    public:
        Application( const ApplicationSpecification& specification );
        void Run();

        virtual ~Application();

        virtual void OnInit()                        = 0;
        virtual void OnShutdown()                    = 0;
        virtual void OnUpdate( Common::Timestep ts ) = 0;

        void PushLayer( Common::Layer* layer );
        void PopLayer( Common::Layer* layer );

        const Common::Memory::Shared<Common::Window>& GetWindow() const
        {
            return m_Window;
        }

    public:
        static Application& GetInstance()
        {
            return *s_Instance;
        }

    private:
    private:
        bool OnClose( Common::EventWindowClose& e )
        {
            return true;
        }
        void ProcessEvents( Common::Event& e );

    private:
        Common::Memory::Shared<Common::Window> m_Window;
        Common::LayerStack                     m_LayerStack;
        ImGuiLayer*                            m_ImGuiLayer;

    private:
        static Application* s_Instance;
        Common::Timestep    m_Timestep;
        float               m_LastFrameTime = 0.0f;
        uint32_t            m_FrameCount    = 0;

        bool m_Run;
    };

    // Iml. by client
    Application* CreateApplication( int argc, char** argv );
} // namespace Radiant