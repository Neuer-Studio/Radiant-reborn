#pragma once

#include <Radiant/Rendering/Scene/Components.hpp>

namespace Radiant
{
	class Entity
	{
	public:
		Entity(const std::string& name = "Entity");
		~Entity();
	private:
		Math::UUID m_UUID;
		std::unordered_map<ComponentType, Component*> m_Components;

		inline static std::vector<Math::UUID> s_UUIDs;
	};
}
