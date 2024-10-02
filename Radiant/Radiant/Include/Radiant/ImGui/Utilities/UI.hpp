#pragma once

#include "imgui/imgui.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Radiant
{
	class UI
	{
	public:
		static void PushID();
		static void PopID();
		static void BeginPropertyGrid();
		static bool Property(const char* label, bool& value);
		static bool Property(const char* label, float& value, float delta = 0.1f, float min = 0.0f, float max = 0.0f);
		static bool PropertyColor(const char* label, glm::vec3& value);
		static bool PropertySlider(const char* label, glm::vec3& value, float min, float max);
		static bool PropertySlider(const char* label, int& value, int min, int max);
		static bool PropertySlider(const char* label, float& value, float min, float max);
		static bool DragFloat(const char* label, float& value, float speed, float min, float max);
		static void EndPropertyGrid();
		static void Image(void* textureID, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& tint_col, const ImVec4& border_col);
		static bool Button(const char* label, const ImVec2& size = ImVec2(0, 0));
		static bool PropertyGridHeader(const std::string& name, bool openByDefault = true);
		static void ShiftCursorY(float distance);
		static bool BeginTreeNode(const char* name, bool defaultOpen);
		static void EndTreeNode();
	};

}