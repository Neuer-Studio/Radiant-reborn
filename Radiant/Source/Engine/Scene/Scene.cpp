#include <Radiant/Scene/Scene.hpp>
#include <Radiant/Scene/Entity.hpp>
#include <Radiant/Rendering/Animation/Skeleton.hpp> //TODO: remove and use component system for bone transform

#include <Radiant/Rendering/SceneRendering.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <Radiant/Core/Serialization/Serialization.hpp>

namespace Radiant
{
    namespace
    {
        template <typename T>
        static std::optional<T> GetData( const Deserialization::NodeReader& node, const std::string& nodeName,
                                         const std::string& valueName )
        {
            const auto nodeData = node.GetChildNodes( nodeName );

            for ( const auto& nodeIT : *nodeData )
            {
                const auto value = nodeIT.GetValue( valueName );
                if ( value )
                {
                    T v = std::any_cast<T>( *value );

                    return v;
                }
            }

            return std::nullopt;
        }

        static std::optional<std::filesystem::path> GetAssetPath( const Deserialization::NodeReader& node )
        {
            return GetData<std::string>( node, "AssetPath", "Asset" );
        }


        static std::optional<std::string> GetTag( const Deserialization::NodeReader& node )
        {
            return GetData<std::string>( node, "TagComponent", "Tag" );
        }

        static Entity CreateEntity( Memory::Weak<Scene> scene, const Deserialization::NodeReader& node, const std::any& value )
        {
            uint64_t uuidValue = std::stoull( std::any_cast<std::string>( value ) );
            UUID     uuid( uuidValue );
            auto&    e = scene->CreateEntityWithID( uuid, GetTag(node).value() );

            return e;
        }

    } // namespace

    Scene::Scene( const std::string& sceneName ) : m_SceneName( sceneName )
    {
    }

    Scene::~Scene()
    {
        GetSerializationSceneString();
    }

    [[nodiscard]] Entity Scene::CreateEntity( const std::string& name /*= ""*/ )
    {
        return CreateChildEntity( std::nullopt, name );
    }

    [[nodiscard]] Entity Scene::CreateEntityWithID( const UUID& uuid, const std::string& name )
    {
        auto  entity      = Entity{ m_Registry.create(), this };
        auto& idComponent = entity.AddComponent<IDComponent>();
        idComponent.ID    = uuid; // NOTE: maybe use std::move?

        entity.AddComponent<TransformComponent>();
        if ( !name.empty() )
            entity.AddComponent<TagComponent>( name );

        entity.AddComponent<RelationshipComponent>();

        RADIANT_VERIFY( m_EntityIDMap.find( uuid ) == m_EntityIDMap.end() );
        m_EntityIDMap[uuid] = entity;

        return entity;
    }

    [[nodiscard]] Entity Scene::CreateChildEntity( const std::optional<Entity>& parent,
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

        auto envMap = m_Registry.group<EnvironmentMapComponent>( entt::get<TransformComponent> );
        for ( auto entity : envMap )
        {
            auto [transformComponent, envMapComponent] =
                 envMap.get<TransformComponent, EnvironmentMapComponent>( entity );
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
                SubmitMesh( mesh, GetModelSpaceBoneTransforms( meshComponent.BoneEntityIds, mesh ),
                            transformComponent.GetTransform() );
            }
        }
        UpdateAnimation( information.TimeStep );

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

    void Scene::SubmitMesh( const Memory::Shared<Mesh>&                  mesh,
                            const std::optional<std::vector<glm::mat4>>& boneTransforms,
                            const glm::mat4&                             transform ) const
    {
        if ( mesh->IsRigged() && boneTransforms )
        {
            auto& boneTransformsValue = boneTransforms.value();
            SceneRendering::Get().SubmitAnimatedMesh( mesh, boneTransformsValue, transform );
        }
        else
        {
            SceneRendering::Get().SubmitStaticMesh( mesh, transform );
        }
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

    std::string Scene::GetSerializationSceneString()
    {
        return SceneSerialize::GetSerializedScene_STRING( this );
    }

    std::optional<std::vector<glm::mat4>>
    Scene::GetModelSpaceBoneTransforms( const std::vector<UUID>&            boneEntityIds,
                                        const Memory::Shared<AnimatedMesh>& mesh )
    {
        // std::vector<glm::mat4> boneTransforms( mesh->GetBoneInfo().size() );
        std::vector<glm::mat4> boneTransforms( 100, glm::mat4( 1.0 ) );

        if ( mesh->IsRigged() )
        {
            const auto& skeleton = mesh->GetSkeleton();
            RADIANT_VERIFY( boneEntityIds.size() == skeleton.BoneCount(),
                            "Wrong number of boneEntityIds for mesh skeleton!" );

            for ( auto i = 0; i < skeleton.BoneCount(); ++i )
            {
                const auto boneEntity     = TryGetEntityWithUUID( boneEntityIds[i] );
                glm::mat4  localTransform = boneEntity
                                                 ? boneEntity->GetComponent<TransformComponent>().GetTransform()
                                                 : glm::identity<glm::mat4>();

                auto parentIndex = skeleton.GetParentBoneIndex( i );
                boneTransforms[i] =
                     ( !parentIndex.has_value() ) ? localTransform : boneTransforms[*parentIndex] * localTransform;
            }
        }
        return boneTransforms;
    }

    void Scene::UpdateAnimation( Timestep ts )
    {
        const auto& view =
             GetAllEntitiesWith<MeshComponent>(); // TODO: Use AnimationComponent instead of MeshComponent
        for ( const auto& entity : view )
        {
            Entity e    = { entity, this };
            auto&  anim = e.GetComponent<MeshComponent>();
            if ( !anim.Mesh || !anim.Mesh->IsRigged() )
            {
                continue;
            }
            const auto& animationController = anim.Mesh.As<AnimatedMesh>()->GetAnimationController();
            animationController->OnUpdate( ts ); // TODO: get from AnimationComponent

            for ( size_t i = 0; i < anim.BoneEntityIds.size(); ++i )
            {
                auto boneTransformEntity = TryGetEntityWithUUID( anim.BoneEntityIds[i] );
                if ( boneTransformEntity )
                {
                    // Note: we're assuming there is always a transform component
                    auto& transform       = boneTransformEntity->GetComponent<TransformComponent>();
                    transform.Translation = animationController->GetTranslation( i );
                    transform.Rotation    = glm::eulerAngles( animationController->GetRotation( i ) );
                    transform.Scale       = animationController->GetScale( i );
                }
            }
        }
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
                boneEntityIds.emplace_back( e ? e->GetUUID() : UUID( 0 ) );
            }
        }
        return boneEntityIds;
    }

    serialized_str SceneSerialize::GetSerializedScene_STRING( Memory::Weak<Scene> scene )
    {
        Serialization::YamlWriter        writer;
        std::vector<Serialization::Node> entities;
        writer.AddValue( "Scene", scene->GetSceneName() );

        const auto& allMeshEntitys = scene->GetAllEntitiesWith<MeshComponent>();
        for ( const auto& e : allMeshEntitys )
        {
            Entity entity( e, scene.Raw() );

            const auto& uuid = entity.GetUUID();
            const auto& mesh = entity.GetComponent<MeshComponent>();
            const auto& tag  = entity.GetComponent_S<TagComponent>();

            const std::string tagStr = tag.has_value() ? tag.value().get() : std::string( "" );

            Serialization::Node nodeEntity;
            nodeEntity.AddValue( "Mesh", uuid.ToString() );
            Serialization::Node tagComponent1;
            tagComponent1.AddValue( "Tag", tagStr );
            nodeEntity.AddChildNode( "TagComponent", tagComponent1 );
            Serialization::Node assetPath;
            assetPath.AddValue( "Asset", mesh.Mesh->GetAssetPath().string() );
            nodeEntity.AddChildNode( "AssetPath", assetPath );

            entities.push_back( nodeEntity );
        }

        const auto& skybox = scene->GetAllEntitiesWith<EnvironmentMapComponent>();
        for ( const auto& e : skybox )
        {
            Entity entity( e, scene.Raw() );

            const auto& uuid = entity.GetUUID();
            const auto& mesh = entity.GetComponent<EnvironmentMapComponent>();
            const auto& tag  = entity.GetComponent_S<TagComponent>();

            const std::string tagStr = tag.has_value() ? tag.value().get() : std::string( "" );

            Serialization::Node nodeEntity;
            nodeEntity.AddValue( "Environment", uuid.ToString() );
            Serialization::Node tagComponent1;
            tagComponent1.AddValue( "Tag", tagStr );
            nodeEntity.AddChildNode( "TagComponent", tagComponent1 );
            Serialization::Node assetPath;
            assetPath.AddValue( "Asset", mesh.SceneEnvironment.FilePath );
            nodeEntity.AddChildNode( "AssetPath", assetPath );

            entities.push_back( nodeEntity );
        }

        writer.AddArray( "Entities", entities );

        //  writer.SaveToFile( "output.yaml" );

        std::cout << "Data saved to output.yaml" << std::endl;

        return "";
    }

    //******************************************************************//
    //******************************************************************//
    //******************************************************************//

    // In the future this function will be much more concise! At the moment this is all BETA version, we need
    // metadata for correct processing, as well as macros that will help to serialize it all.
    std::optional<Radiant::Memory::Shared<Radiant::Scene>>
    SceneDeserialize::GetDeserializedScene_Object( const serialized_str& context )
    {
        Deserialization::NodeReader reader;
        reader.LoadFromFile( "output.yaml" );

        const auto& sceneYAML = reader.GetValue( "Scene" );
        if ( sceneYAML )
        {
            auto scene = MAKE_SHARED_OBJECT( Scene, std::any_cast<std::string>( *sceneYAML ) );

            auto entities = reader.GetChildNodes( "Entities" );
            RA_DEBUG( "Number of entities: {}", entities->size() );

            for ( const auto& entity : *entities )
            {
                {
                    const auto meshValue = entity.GetValue( "Mesh" );
                    if ( meshValue )
                    {
                        auto  e                = CreateEntity( scene.Raw(), entity, *meshValue );
                        auto& component        = e.AddComponent<MeshComponent>();
                        component.LoadAsStatic = true;

                        const auto assetPath = GetAssetPath( entity );

                        if ( assetPath )
                        {
                            component.Mesh = MAKE_SHARED_OBJECT( StaticMesh, *assetPath );
                        }

                        continue;
                    }
                }

                {
                    const auto envValue = entity.GetValue( "Environment" );

                    if ( envValue )
                    {
                        auto  e                = CreateEntity( scene.Raw(), entity , *envValue );
                        auto& component        = e.AddComponent<EnvironmentMapComponent>();

                        const auto assetPath = GetAssetPath( entity );

                        if ( assetPath )
                        {
                            component.SceneEnvironment = Environment::Create(*assetPath);
                        }

                        continue;
                    }
                }
            }

            return scene;
        }

        return std::nullopt;
    }

} // namespace Radiant