#include <Radiant/ImGui/Utilities/UI.hpp>

#include <Radiant/Utilities/String.hpp>

namespace Radiant
{
	int s_UIContextID = 0;
	uint32_t s_Counter = 0;
	char s_IDBuffer[16];

	void UI::PushID()
	{
		ImGui::PushID(s_UIContextID++);
		s_Counter = 0;
	}

	void UI::PopID()
	{
		ImGui::PopID();
		s_UIContextID--;
	}

	void UI::BeginPropertyGrid()
	{
		PushID();
		ImGui::Columns(2);
	}

	bool UI::Property(const char* label, bool& value)
	{
		bool modified = false;

		ImGui::Text(label);
		ImGui::NextColumn();
		ImGui::PushItemWidth(-1);

		s_IDBuffer[0] = '#';
		s_IDBuffer[1] = '#';
		memset(s_IDBuffer + 2, 0, 14);
		_itoa(s_Counter++, s_IDBuffer + 2, 16);
		if (ImGui::Checkbox(s_IDBuffer, &value))
			modified = true;

		ImGui::PopItemWidth();
		ImGui::NextColumn();

		return modified;
	}

	bool UI::Property(const char* label, float& value, float delta, float min, float max)
	{
		bool modified = false;

		ImGui::Text(label);
		ImGui::NextColumn();
		ImGui::PushItemWidth(-1);

		s_IDBuffer[0] = '#';
		s_IDBuffer[1] = '#';
		memset(s_IDBuffer + 2, 0, 14);
		_itoa(s_Counter++, s_IDBuffer + 2, 16);
		if (ImGui::DragFloat(s_IDBuffer, &value, delta, min, max))
			modified = true;

		ImGui::PopItemWidth();
		ImGui::NextColumn();

		return modified;
	}

	bool UI::PropertyColor(const char* label, glm::vec3& value)
	{
		bool modified = false;

		ImGui::Text(label);
		ImGui::NextColumn();
		ImGui::PushItemWidth(-1);

		s_IDBuffer[0] = '#';
		s_IDBuffer[1] = '#';
		memset(s_IDBuffer + 2, 0, 14);
		itoa(s_Counter++, s_IDBuffer + 2, 16);
		if (ImGui::ColorEdit3(s_IDBuffer, glm::value_ptr(value)))
			modified = true;

		ImGui::PopItemWidth();
		ImGui::NextColumn();

		return modified;
	}

	bool UI::PropertySlider(const char* label, glm::vec3& value, float min, float max)
	{
		bool modified = false;

		ImGui::Text(label);
		ImGui::NextColumn();
		ImGui::PushItemWidth(-1);

		s_IDBuffer[0] = '#';
		s_IDBuffer[1] = '#';
		memset(s_IDBuffer + 2, 0, 14);
		_itoa(s_Counter++, s_IDBuffer + 2, 16);
		if (ImGui::SliderFloat3(s_IDBuffer, glm::value_ptr(value), min, max))
			modified = true;

		ImGui::PopItemWidth();
		ImGui::NextColumn();

		return modified;
	}

	bool UI::PropertySlider(const char* label, int& value, int min, int max)
	{
		bool modified = false;

		ImGui::Text(label);
		ImGui::NextColumn();
		ImGui::PushItemWidth(-1);

		s_IDBuffer[0] = '#';
		s_IDBuffer[1] = '#';
		memset(s_IDBuffer + 2, 0, 14);
		itoa(s_Counter++, s_IDBuffer + 2, 16);
		if (ImGui::SliderInt(s_IDBuffer, &value, min, max))
			modified = true;

		ImGui::PopItemWidth();
		ImGui::NextColumn();

		return modified;
	}

	bool UI::PropertySlider(const char* label, float& value, float min, float max)
	{
		bool modified = false;

		ImGui::Text(label);
		ImGui::NextColumn();
		ImGui::PushItemWidth(-1);

		s_IDBuffer[0] = '#';
		s_IDBuffer[1] = '#';
		memset(s_IDBuffer + 2, 0, 14);
		_itoa(s_Counter++, s_IDBuffer + 2, 16);
		if (ImGui::SliderFloat(s_IDBuffer, &value, min, max))
			modified = true;

		ImGui::PopItemWidth();
		ImGui::NextColumn();

		return modified;
	}

	bool UI::DragFloat(const char* label, float& value, float speed, float min, float max)
	{
		bool modified = false;

		ImGui::Text(label);
		ImGui::NextColumn();
		ImGui::PushItemWidth(-1);

		s_IDBuffer[0] = '#';
		s_IDBuffer[1] = '#';
		memset(s_IDBuffer + 2, 0, 14);
		_itoa(s_Counter++, s_IDBuffer + 2, 16);
		if (ImGui::DragFloat(s_IDBuffer, &value, speed, min, max))
			modified = true;

		ImGui::PopItemWidth();
		ImGui::NextColumn();

		return modified;
	}

	void UI::EndPropertyGrid()
	{
		ImGui::Columns(1);
		PopID();
	}

	void UI::Image(void* textureID, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& tint_col, const ImVec4& border_col)
	{
		ImGui::Image(textureID, size, uv0, uv1, tint_col, border_col);
	}

	bool Button(const char* label, const ImVec2& size = ImVec2(0, 0))
	{
		bool result = ImGui::Button(label, size);
		ImGui::NextColumn();
		return result;
	}


	bool PropertyGridHeader(const std::string& name, bool openByDefault = true)
	{
		ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_Framed
			| ImGuiTreeNodeFlags_SpanAvailWidth
			| ImGuiTreeNodeFlags_AllowItemOverlap
			| ImGuiTreeNodeFlags_FramePadding;

		if (openByDefault)
			treeNodeFlags |= ImGuiTreeNodeFlags_DefaultOpen;

		bool open = false;
		const float framePaddingX = 6.0f;
		const float framePaddingY = 6.0f; // affects height of the header

		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ framePaddingX, framePaddingY });

		//UI::PushID();
		ImGui::PushID(name.c_str());
		open = ImGui::TreeNodeEx("##dummy_id", treeNodeFlags, name.c_str());
		//UI::PopID();
		ImGui::PopID();

		ImGui::PopStyleVar();
		ImGui::PopStyleVar();
		return open;
	}

	void UI::ShiftCursorY(float distance)
	{
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + distance);
	}

	bool UI::BeginTreeNode(const char* name, bool defaultOpen)
	{
		ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding;
		if (defaultOpen)
			treeNodeFlags |= ImGuiTreeNodeFlags_DefaultOpen;

		return ImGui::TreeNodeEx(name, treeNodeFlags);
	}

	void UI::EndTreeNode()
	{
		ImGui::TreePop();
	}
}