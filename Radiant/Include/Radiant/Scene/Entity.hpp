#pragma once

#include <Radiant/Scene/Component.hpp>

#include <entt/entt.hpp>

namespace Radiant
{
	class Scene;
	class Entity
	{
	public:
		Entity() = default;
		Entity(entt::entity handle, Scene* scene)
			: m_EntityHandle(handle), m_Scene(scene) {}

		~Entity();

		template<typename T>
		bool HasComponent()
		{
			return m_Scene->m_Registry.has<T>(m_EntityHandle);
		}

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args)
		{
			RADIANT_VERIFY(!HasComponent<T>());
			return m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
		}

		template<typename T>
		T& GetComponent()
		{
			RADIANT_VERIFY(HasComponent<T>());
			return m_Scene->m_Registry.get<T>(m_EntityHandle);
		}

		template<typename T>
		void RemoveComponent()
		{
			RADIANT_VERIFY(HasComponent<T>(), "Entity doesn't have component!");
			m_Scene->m_Registry.remove<T>(m_EntityHandle);
		}

		bool operator==(const Entity& other) const
		{
			return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene;
		}
		operator bool() const { return m_Scene; }
	private:
		entt::entity m_EntityHandle{ entt::null };
		Scene* m_Scene;

		friend class Scene;
	};
}
