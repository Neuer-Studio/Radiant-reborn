#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include <Radiant/Rendering/Mesh.hpp>
#include <Radiant/Core/Camera.hpp>

#include <Radiant/Rendering/Environment.hpp>
#include <Common/Core/Serialization/SerializationProvider.hpp>

namespace Radiant
{
    struct IDComponent
    {
        Common::UUID ID;
    };

    struct TagComponent
    {
        std::string Tag;

        TagComponent()                            = default;
        TagComponent( const TagComponent& other ) = default;
        TagComponent( const std::string& tag ) : Tag( tag )
        {
        }

        operator std::string&()
        {
            return Tag;
        }
        operator const std::string&() const
        {
            return Tag;
        }
    };

    struct TransformComponent
    {
        glm::vec3 Translation = { 0.0f, 0.0f, 0.0f };
        glm::vec3 Rotation    = { 0.0f, 0.0f, 0.0f };
        glm::vec3 Scale       = { 1.0f, 1.0f, 1.0f };

        TransformComponent()                                  = default;
        TransformComponent( const TransformComponent& other ) = default;
        TransformComponent( const glm::vec3& translation ) : Translation( translation )
        {
        }

        glm::mat4 GetTransform() const
        {
            return glm::translate( glm::mat4( 1.0f ), Translation ) * glm::toMat4( glm::quat( Rotation ) ) *
                   glm::scale( glm::mat4( 1.0f ), Scale );
        }
    };

    struct MeshComponent SERIALIZABLE_CLASS_MAKE
    {
        Common::Memory::Shared<class Mesh> Mesh;

        bool              LoadAsStatic = false; // TODO: remove
        std::vector<Common::UUID> BoneEntityIds;

        MeshComponent() = default;
        MeshComponent( const Common::Memory::Shared<class StaticMesh>& mesh )
             : Mesh( mesh )
        {
        }

        operator Common::Memory::Shared<Radiant::StaticMesh>()
        {
            return Mesh;
        }
    };

    struct RelationshipComponent
    {
        std::optional<Common::UUID> ParentHandle;
        std::vector<Common::UUID>   Children;

        RelationshipComponent()                                     = default;
        RelationshipComponent( const RelationshipComponent& other ) = default;
        RelationshipComponent(Common::UUID parent ) : ParentHandle( parent )
        {
        }
    };

    struct EnvironmentMapComponent
    {
        Environment SceneEnvironment;
        float       Intensity         = 1.0f;
        float       EnvironmentMapLod = 0.0f;
    };

    struct CameraComponent
    {
        class Camera Camera;
        bool         Primary = true;

        CameraComponent()                               = default;
        CameraComponent( const CameraComponent& other ) = default;

        operator class Camera &()
        {
            return Camera;
        }
        operator const class Camera &() const
        {
            return Camera;
        }
    };

    struct DirectionalLightComponent
    {
        glm::vec3 Radiance    = glm::vec3( 1.0f );
        float     Intensity   = 1.0f;
        bool      CastShadows = true;
    };

    struct PointLightComponent
    {
        glm::vec3 Radiance = glm::vec3( 1.0f );

        float Intensity = 1.0f;
        float Radius    = 10.0f;
        float Falloff   = 1.0f;
        float LightSize = 2.5f;
    };

} // namespace Radiant