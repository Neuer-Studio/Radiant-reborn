#pragma once

#include <Radiant/ImGui/ImGuiLayer.hpp>

namespace Radiant
{
    class OpenGLImGuiLayer final : public ImGuiLayer
    {
    public:
        OpenGLImGuiLayer();
        OpenGLImGuiLayer( const std::string& name );
        virtual ~OpenGLImGuiLayer();

        virtual void Begin() override;
        virtual void End() override;

        virtual void OnAttach() override;
        virtual void OnDetach() override;
        virtual void OnUpdate( Common::Timestep ts ) override;
        virtual void OnImGuiRender() override;
        virtual void OnEvent( Common::Event& e ) override
        {
        }

    private:
        float m_Time = 0.0f;
    };
} // namespace Radiant