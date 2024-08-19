#pragma once

#include <Radiant/Scene/Scene.hpp>
#include <Radiant/Scene/Entity.hpp>

namespace Radiant
{
	class SceneHierarchyPanel final : public Memory::RefCounted
	{
	public:
		SceneHierarchyPanel(const Memory::Shared<Scene>& scene = nullptr);
		void SetContext(const Memory::Shared<Scene>& scene);

		void DrawComponentsUI(const std::string& ButtonName = "ADD", float x = 40, float y = 25);
		void DrawImGuiUI();
	private:
		void DrawEntityNodeUI(Entity entity);
		void DrawPropertiesUI(Entity entity);
	private:
		Entity m_SelectionContext{};
		Memory::Shared<Scene> m_Context;
	};
}