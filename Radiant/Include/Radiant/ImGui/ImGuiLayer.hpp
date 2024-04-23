#pragma once

#include <Radiant/Core/Layer.hpp>

namespace Radiant
{
	class ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer(const std::string& name = "ImGui")
			: Layer(name)
		{}

		virtual void Begin() = 0;
		virtual void End() = 0;

		void SetDarkThemeColors();

		static ImGuiLayer* Create();
		static ImGuiLayer* Create(const std::string& name);
	};
}