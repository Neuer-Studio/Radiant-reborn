#pragma once

#include <Radiant/Core/Camera.hpp>
#include <Radiant/Rendering/Mesh.hpp>

#include <entt/entt.hpp>

namespace Radiant
{
    struct Environment;
    struct SceneRendering;
    class Entity;

    struct DirectionalLight
    {
        glm::vec3 Direction;
        alignas( 16 ) glm::vec3 Radiance; // NOTE: GLSL interprets vec3 (12bytes) as vec4 (16bytes)

        float Intensity;
        bool  CastShadows;
    };

    struct PointLight
    {
        glm::vec3 Direction;
        alignas( 16 ) glm::vec3 Radiance; // NOTE: GLSL interprets vec3 (12bytes) as vec4 (16bytes)

        float Intensity;
        float Radius;
        float Falloff;
        float LightSize;
    };

    struct LightEnvironment
    {
        DirectionalLight        DirectionalLights;
        std::vector<PointLight> PointLights;

        [[nodiscard]] uint32_t GetPointLightsSize() const
        {
            return (uint32_t)( PointLights.size() * sizeof( PointLight ) );
        }
    };

    struct SceneUpdateInformation
    {
        Common::Timestep TimeStep;
        Camera           Camera;
        uint32_t         Width;
        uint32_t         Height;
    };

    struct SceneOptions
    {
        bool ShowGrid = true;
        bool ShowAABB = true;
    };

    class Entity;
    using EntityMap = std::unordered_map<Common::UUID, Entity>;

    class Scene : public Common::Memory::RefCounted
    {
    public:
        Scene( const std::string& sceneName );
        ~Scene();

        std::string GetSceneName() const
        {
            return m_SceneName;
        }

        [[nodiscard]] Entity CreateEntity( const std::string& name = "" );
        [[nodiscard]] Entity CreateEntityWithID( const Common::UUID& uuid, const std::string& name );
        [[nodiscard]] Entity CreateChildEntity( const std::optional<Entity>& parent,
                                                const std::string&           name = "" );

        void BuildMeshBoneEntityIds( Entity& parentEntity ); // TODO

        [[nodiscard]] std::optional<Radiant::Entity> TryGetDescendantEntityWithTag( Entity&            entity,
                                                                                    const std::string& tag );

        std::vector<Common::UUID> Scene::FindBoneEntityIds( Entity&                                     parent,
                                                            const Common::Memory::Shared<AnimatedMesh>& mesh );

        std::optional<Radiant::Entity> TryGetEntityWithUUID( const Common::UUID& uuid ) const;

        [[nodiscard]] Entity GetMainCameraEntity();

        void                           OnUpdate( const SceneUpdateInformation& information );
        void                           SetEnvironment( const Environment& env );
        [[nodiscard]] Environment      CreateEnvironmentScene( const std::filesystem::path& filepath ) const;
        [[nodiscard]] LightEnvironment GetLightEnvironment() const
        {
            return m_LightEnvironment;
        }
        const SceneOptions GetSceneOptions() const
        {
            return m_Options;
        }

        const auto& GetSceneUpdateInfo() const
        {
            return m_Information;
        }

        inline const uint32_t GetSceneSamplesCount() const
        {
            return m_SamplesCount;
        }

        Common::UUID GetUUID() const
        {
            return m_SceneID;
        }

        void                                   SubmitMesh( const Common::Memory::Shared<Mesh>&          mesh,
                                                           const std::optional<std::vector<glm::mat4>>& boneTransforms,
                                                           const glm::mat4&                             transform ) const;
        const Common::Memory::Shared<Image2D>& GetFinalPassImage() const;
        void                                   SetEnvMapRotation( float rotation );
        void                                   SetIBLContribution( float value );

        [[nodiscard]] Entity InstantiateMesh( const Common::Memory::Shared<Mesh>& mesh,
                                              const std::optional<Entity>&        parentEntity );
        void                 BuildMeshEntityHierarchy( const Entity&                               rootEntity,
                                                       const Common::Memory::Shared<AnimatedMesh>& mesh );

        template <typename... Components>
        auto GetAllEntitiesWith()
        {
            return m_Registry.view<Components...>();
        }

        template <typename T>
        auto GetAllComponentsOfType()
        {
            std::vector<T*> components;
            m_Registry.view<T>().each( [&components]( auto entity, T& component )
                                       { components.push_back( &component ); } );

            return components;
        }

        std::string GetSerializationSceneString();

    private:
        [[nodiscard]] std::optional<std::vector<glm::mat4>>
        GetModelSpaceBoneTransforms( const std::vector<Common::UUID>&                    boneEntityIds,
                                     const Common::Memory::Shared<AnimatedMesh>& mesh );

        void UpdateAnimation( Common::Timestep ts );

    private:
        Common::UUID m_SceneID;
        EntityMap    m_EntityIDMap;

        SceneOptions m_Options;
        uint32_t     m_SamplesCount = 2;

        std::string    m_SceneName;
        entt::registry m_Registry;

        LightEnvironment       m_LightEnvironment;
        SceneUpdateInformation m_Information;

        friend class Entity;
        friend class SceneHierarchyPanel;
        friend class SceneRenderingPanel;
    };

    class SceneSerialize
    {
    public:
        static Common::serialized_str GetSerializedScene_STRING( Common::Memory::Weak<Scene> scene );
    };

    class SceneDeserialize
    {
    public:
        static std::optional<Common::Memory::Shared<Radiant::Scene>>
        GetDeserializedScene_Object( const Common::serialized_str& context );
    };
} // namespace Radiant