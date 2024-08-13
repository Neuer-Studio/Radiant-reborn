#include <Radiant/Scene/Scene.hpp>
#include <Radiant/Scene/Entity.hpp>
#include <Radiant/Rendering/Animation/Skeleton.hpp> //TODO: remove and use component system for bone transform

#include <Radiant/Scene/SceneRendering.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Radiant
{

    Scene::Scene( const std::string& sceneName ) : m_SceneName( sceneName )
    {
    }

    Scene::~Scene()
    {
    }

    Radiant::Entity Scene::CreateEntity( const std::string& name /*= ""*/ )
    {
        return CreateChildEntity( std::nullopt, name );
    }

    Radiant::Entity Scene::CreateChildEntity( const std::optional<Entity>& parent,
                                              const std::string&           name /*= "" */ )
    {
        auto  entity      = Entity{ m_Registry.create(), this };
        auto& idComponent = entity.AddComponent<IDComponent>();
        idComponent.ID    = UUID();

        entity.AddComponent<TransformComponent>();
        if ( !name.empty() )
        {
            entity.AddComponent<TagComponent>( name );
        }

        entity.AddComponent<RelationshipComponent>();
        if ( parent )
        {
            entity.SetParent( *parent );
        }

        m_EntityIDMap[idComponent.ID] = entity;
        return entity;
    }

    std::optional<Radiant::Entity> Scene::TryGetDescendantEntityWithTag( Entity& entity, const std::string& tag )
    {
        if ( entity )
        {
            if ( entity.GetComponent<TagComponent>().Tag == tag )
                return entity;

            for ( const auto& childId : entity.Children() )
            {
                const auto& descendant =
                     TryGetDescendantEntityWithTag( *( TryGetEntityWithUUID( childId ) ), tag );
                if ( descendant )
                    return descendant;
            }
        }
        return std::nullopt;
    }

    void Scene::BuildMeshBoneEntityIds( Entity& parentEntity )
    {
        if ( parentEntity.HasComponent<MeshComponent>() )
        {
            auto& mc   = parentEntity.GetComponent<MeshComponent>();
            auto  mesh = mc.Mesh;
            if ( mesh && mesh->IsRigged() )
            {
                mc.BoneEntityIds = FindBoneEntityIds( parentEntity, mesh.As<AnimatedMesh>() );
            }
        }
    }

    std::optional<Radiant::Entity> Scene::TryGetEntityWithUUID( const UUID& uuid ) const
    {
        if ( const auto iter = m_EntityIDMap.find( uuid ); iter != m_EntityIDMap.end() )
            return iter->second;

        return std::nullopt;
    }

    Entity Scene::GetMainCameraEntity()
    {
        auto view = m_Registry.view<CameraComponent>();
        for ( const auto& entity : view )
        {
            auto& comp = view.get<CameraComponent>( entity );
            if ( comp.Primary )
                return { entity, this };
        }
        return {};
    }

    void Scene::OnUpdate( const SceneUpdateInformation& information )
    {
        /*	Entity& cameraEntity = GetMainCameraEntity();
                if (!cameraEntity)
                        return;*/

        auto dirLight = m_Registry.group<DirectionalLightComponent>( entt::get<TransformComponent> );
        for ( auto entity : dirLight )
        {
            auto [transformComponent, lightComponent] =
                 dirLight.get<TransformComponent, DirectionalLightComponent>( entity );
            glm::vec3 direction =
                 -glm::normalize( glm::mat3( transformComponent.GetTransform() ) * glm::vec3( 1.0f ) );
            m_LightEnvironment.DirectionalLights = { direction, lightComponent.Radiance, lightComponent.Intensity,
                                                     lightComponent.CastShadows };
        }

        // Point lights
        {
            auto     pointLights     = m_Registry.group<PointLightComponent>( entt::get<TransformComponent> );
            uint32_t pointLightIndex = 0;
            for ( auto entity : pointLights )
            {
                auto [transformComponent, lightComponent] =
                     pointLights.get<TransformComponent, PointLightComponent>( entity );
                glm::vec3 direction = transformComponent.Translation; // TODO: flag paneloutliner only translation
                m_LightEnvironment.PointLights.resize( pointLights.size() );
                m_LightEnvironment.PointLights[pointLightIndex++] = {
                     direction,
                     lightComponent.Radiance,
                     lightComponent.Intensity,
                     lightComponent.Radius,
                     lightComponent.Falloff,
                     lightComponent.LightSize,
                };
            }
        }

        auto envMap = m_Registry.group<EnvironmentMap>( entt::get<TransformComponent> );
        for ( auto entity : envMap )
        {
            auto [transformComponent, envMapComponent] = envMap.get<TransformComponent, EnvironmentMap>( entity );
            EnvironmentAttributes attrs;
            attrs.EnvironmentMapLod = envMapComponent.EnvironmentMapLod;
            attrs.Intensity         = envMapComponent.Intensity;

            SceneRendering::Get().SetEnvironmentAttributes( attrs );
        }

        auto meshs = m_Registry.group<MeshComponent>( entt::get<TransformComponent> );
        for ( auto entity : meshs )
        {
            auto [transformComponent, meshComponent] = meshs.get<TransformComponent, MeshComponent>( entity );
            if ( meshComponent.Mesh )
            {
                const auto& mesh = meshComponent.Mesh;
                if ( mesh->IsRigged() )
                {
                    mesh.As<AnimatedMesh>()->GetAnimationController()->OnUpdate( information.TimeStep );
                    SceneRendering::Get().SubmitAnimatedMesh( mesh, GetModelSpaceBoneTransforms( mesh ).value(),
                                                              transformComponent.GetTransform() );
                }
                else
                {
                    SceneRendering::Get().SubmitStaticMesh( mesh, transformComponent.GetTransform() );
                }
            }
        }

        SceneRendering::Get().BeginScene( this, information.Camera );
        SceneRendering::Get().OnUpdate( information.TimeStep );
        SceneRendering::Get().SetSceneVeiwPortSize( { information.Width, information.Height } );
        SceneRendering::Get().EndScene();
    }

    void Scene::SetEnvironment( const Environment& env )
    {
        SceneRendering::Get().SetEnvironment( env );
    }

    Environment Scene::CreateEnvironmentScene( const std::filesystem::path& filepath ) const
    {
        return SceneRendering::Get().CreateEnvironmentMap( filepath );
    }

    void Scene::SubmitMesh( const Memory::Shared<Mesh>& mesh, std::vector<glm::mat4>& boneTransforms,
                            const glm::mat4& transform ) const
    {
        SceneRendering::Get().SubmitAnimatedMesh( mesh, boneTransforms, transform );
    }

    const Radiant::Memory::Shared<Radiant::Image2D>& Scene::GetFinalPassImage() const
    {
        return SceneRendering::Get().GetFinalPassImage();
    }

    void Scene::SetEnvMapRotation( float rotation )
    {
        SceneRendering::Get().SetEnvMapRotation( rotation );
    }

    void Scene::SetIBLContribution( float value )
    {
        SceneRendering::Get().SetIBLContribution( value );
    }

    [[nodiscard]] Radiant::Entity Scene::InstantiateMesh( const Memory::Shared<Mesh>&  mesh,
                                                          const std::optional<Entity>& parentEntity )
    {
        const auto& skeleton = mesh.As<AnimatedMesh>()->GetSkeleton();
        if ( !mesh->IsRigged() )
        {
            return parentEntity.value();
        }
        BuildMeshEntityHierarchy( parentEntity.value(), mesh );
        Entity e = *parentEntity;
        BuildMeshBoneEntityIds( e );
    }

    void Scene::BuildMeshEntityHierarchy( const Entity& rootEntity, const Memory::Shared<AnimatedMesh>& mesh )
    {
        const auto& raw_bones = mesh->GetBonesHierarchy_RAW();

        std::vector<Entity> entities( raw_bones.size() );

        for ( uint32_t i = 0; i < raw_bones.size(); ++i )
        {
            const auto& [boneName, parentIndex] = raw_bones[i];

            Entity parentEntity = parentIndex.has_value() ? entities[parentIndex.value()] : rootEntity;
            entities[i]         = CreateChildEntity( parentEntity, boneName );
        }
    }

    std::optional<std::vector<glm::mat4>>
    Scene::GetModelSpaceBoneTransforms( const Memory::Shared<AnimatedMesh>& mesh )
    {
        // std::vector<glm::mat4> boneTransforms( mesh->GetBoneInfo().size() );
        std::vector<glm::mat4> boneTransforms( 100, glm::mat4( 1.0 ) );

        const auto& controller = mesh->GetAnimationController();

        if ( mesh->IsRigged() )
        {
            const auto& skeleton = mesh->GetSkeleton();
            for ( auto i = 0; i < skeleton.BoneCount(); ++i )
            {
                const auto localTransform =
                     glm::translate( glm::mat4( 1.0f ), controller->GetTranslation( i ) ) *
                     glm::toMat4( glm::quat( controller->GetRotation( i ) ) ) *
                     glm::scale( glm::mat4( 1.0f ),
                                 controller->GetScale( i ) ); // TODO: move to component system

                auto parentIndex = skeleton.GetParentBoneIndex( i );
                boneTransforms[i] =
                     ( !parentIndex.has_value() ) ? localTransform : boneTransforms[*parentIndex] * localTransform;
            }
        }
        return boneTransforms;
    }

    std::vector<UUID> Scene::FindBoneEntityIds( Entity& parent, const Memory::Shared<AnimatedMesh>& mesh )
    {
        std::vector<UUID> boneEntityIds;
        // given a parent entity, find descendant entities holding the transforms for the specified mesh's bones
        if ( mesh )
        {
            const auto& bonesInfo = mesh->GetSkeleton().GetBonesInfo();
            for ( const auto& boneInfo : bonesInfo )
            {
                const auto& e = TryGetDescendantEntityWithTag( parent, boneInfo.BoneName );
                boneEntityIds.emplace_back(e ? e->GetUUID() : UUID(0));
            }
        }
        return boneEntityIds;
    }

} // namespace Radiant