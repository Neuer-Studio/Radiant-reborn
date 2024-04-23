#pragma once

#include <Radiant/ImGui/ImGuiLayer.hpp>

namespace Radiant
{
	class OpenGLImGuiLayer final : public ImGuiLayer
	{
	public:
		OpenGLImGuiLayer();
		OpenGLImGuiLayer(const std::string& name);
		virtual ~OpenGLImGuiLayer();

		virtual void Begin() override;
		virtual void End() override;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(Timestep ts) override;
		virtual void OnImGuiRender() override;
		virtual void OnEvent(Event& e) override {}
	private:
		float m_Time = 0.0f;

	};
}