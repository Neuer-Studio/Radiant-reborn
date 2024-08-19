#pragma once

#include <Radiant/Scene/Components.hpp>
#include <Radiant/Scene/Scene.hpp>

#include <entt/entt.hpp>

namespace Radiant
{
    class Entity
    {
    public:
        Entity() = default;
        Entity( entt::entity handle, Scene* scene ) : m_EntityHandle( handle ), m_Scene( scene )
        {
        }

        ~Entity();

        template <typename T>
        bool HasComponent() const
        {
            return m_Scene->m_Registry.has<T>( m_EntityHandle );
        }

        template <typename T>
        bool HasComponent()
        {
            return m_Scene->m_Registry.has<T>( m_EntityHandle );
        }

        template <typename T, typename... Args>
        T& AddComponent( Args&&... args )
        {
            RADIANT_VERIFY( !HasComponent<T>() );
            return m_Scene->m_Registry.emplace<T>( m_EntityHandle, std::forward<Args>( args )... );
        }

        template <typename T>
        const T& GetComponent() const
        {
            RADIANT_VERIFY( HasComponent<T>() );
            return m_Scene->m_Registry.get<T>( m_EntityHandle );
        }

        template <typename T>
        T& GetComponent()
        {
            RADIANT_VERIFY( HasComponent<T>() );
            return m_Scene->m_Registry.get<T>( m_EntityHandle );
        }

        template <typename T>
        void RemoveComponent()
        {
            RADIANT_VERIFY( HasComponent<T>(), "Entity doesn't have component!" );
            m_Scene->m_Registry.remove<T>( m_EntityHandle );
        }

        const UUID GetUUID() const
        {
            return GetComponent<IDComponent>().ID;
        }

        UUID GetUUID()
        {
            return GetComponent<IDComponent>().ID;
        }

        UUID GetSceneUUID()
        {
            return m_Scene->GetUUID();
        }

        std::optional<UUID> GetParentUUID() const
        {
            return GetComponent<RelationshipComponent>().ParentHandle;
        }

        void SetParentUUID( UUID parent )
        {
            GetComponent<RelationshipComponent>().ParentHandle = parent;
        }

        std::vector<UUID>& Children()
        {
            return GetComponent<RelationshipComponent>().Children;
        }

        [[nodiscard]] std::optional<Entity> GetParent()
        {
            const auto& parent = GetParentUUID();
            if ( !parent )
            {
                return std::nullopt;
            }
            return m_Scene->TryGetEntityWithUUID( *parent );
        }

        bool RemoveChild( const Entity& child )
        {
            UUID               childId  = child.GetUUID();
            std::vector<UUID>& children = Children();
            auto               it       = std::find( children.begin(), children.end(), childId );
            if ( it != children.end() )
            {
                children.erase( it );
                return true;
            }

            return false;
        }

        void SetParent( Entity parent );

        [[nodiscard]] std::optional<Entity> TryGetEntityWithUUID( const UUID& uuid ) const;

        bool operator==( const Entity& other ) const
        {
            return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene;
        }
        operator bool() const
        {
            return m_Scene;
        }

    private:
        entt::entity m_EntityHandle{ entt::null };
        Scene*       m_Scene;

        friend class Scene;
    };
} // namespace Radiant
