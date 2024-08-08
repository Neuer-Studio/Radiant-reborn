#pragma once

#include <Radiant/Rendering/VertexBuffer.hpp>
#include <Radiant/Rendering/IndexBuffer.hpp>
#include <Radiant/Rendering/Material.hpp>
#include <Radiant/Core/Math/AABB.hpp>

#include <Radiant/Rendering/Animation/AssimpExporter.hpp>
#include <Radiant/Rendering/Animation/Skeleton.hpp>
#include <Radiant/Rendering/Animation/AnimationController.hpp>

#include <glm/glm.hpp>

#include <Radiant/Rendering/Animation/BoneInfo.hpp>

struct aiNode;
struct aiAnimation;
struct aiNodeAnim;
struct aiScene;
struct aiMesh;

namespace Assimp
{
    class Importer;
}

namespace Radiant
{
    static constexpr uint32_t MAX_BONE_INFLUENCE = 4U;

    struct StaticVertex
    {
        glm::vec3 Position;
        glm::vec3 Normals;
        glm::vec2 TexCoords;
        glm::vec3 Tangent;
        glm::vec3 Bitangent;
    };

    struct AnimatedVertex
    {
        AnimatedVertex()
        {
            SetDataToDefault();
        }

        StaticVertex                          StaticVertexData;
        std::array<int, MAX_BONE_INFLUENCE>   IDs;
        std::array<float, MAX_BONE_INFLUENCE> Weights;

        void SetDataToDefault()
        {
            IDs.fill( -1 );
            Weights.fill( 0 );
        }

        void AddBoneData( int id, float weight )
        {
            for ( uint32_t i = 0; i < MAX_BONE_INFLUENCE; i++ )
            {
                if ( IDs[i] < 0 )
                {
                    IDs[i]     = id;
                    Weights[i] = weight;

                    return;
                }
            }
            RA_WARN( "Vertex has more than four bones/weights affecting it, extra data will be discarded "
                     "(BoneID={0}, Weight={1})",
                     id, weight );
        }
    };

    struct Index
    {
        uint32_t V1, V2, V3;
    };

    enum class TextureType
    {
        None = 0,
        Diffuse,
        Specular,
        Normal,
    };

    struct Submesh
    {
        uint32_t   BaseVertex;
        uint32_t   BaseIndex;
        uint32_t   MaterialIndex;
        uint32_t   IndexCount;
        Math::AABB BoundingBox;

        glm::mat4 Transform;
    };

    class Mesh : public Memory::RefCounted
    {
    public:
        Mesh( const std::filesystem::path& filepath );

        std::vector<Submesh>& GetSubmeshes()
        {
            return m_Submeshes;
        }
        const std::vector<Submesh>& GetSubmeshes() const
        {
            return m_Submeshes;
        }

        const std::string& GetName() const
        {
            return m_Name;
        }

        void     Use() const;
        uint32_t GetIndexCount() const
        {
            return m_IndexBuffer->GetCount();
        }

        const auto& GetAnimationController() const
        {
            return m_AnimationController;
        }

        const auto& GetVertexBuffer() const
        {
            return m_VertexBuffer;
        }
        const auto& GetIndexBuffer() const
        {
            return m_IndexBuffer;
        }

        auto& GetBoneInfo()
        {
            return m_BoneInfo;
        }

        const auto& GetGlobalInverseTransform() const
        {
            return m_GlobalInverseTransform;
        }

    private:
        void ExtractBoneWeightForVertices( std::vector<AnimatedVertex>& vertices, aiMesh* mesh,
                                           const aiScene* scene );
        void TraverseNodes( aiNode* node, const glm::mat4& parentTransform = glm::mat4( 1.0f ),
                            uint32_t level = 0 );

    private:
        glm::mat4 m_GlobalInverseTransform;

        Animation::Skeleton                                  m_Skeleton;
        std::vector<Animation::Animation>                    m_Animations;
        std::unique_ptr<Animation::AnimationController>      m_AnimationController;
        std::unordered_map<std::string, Animation::BoneInfo> m_BoneInfo;

        std::vector<Submesh> m_Submeshes;

        Memory::Shared<VertexBuffer> m_VertexBuffer;
        Memory::Shared<IndexBuffer>  m_IndexBuffer;
        Memory::Shared<Material>     m_Material;

        std::vector<Index> m_Indices;

        std::string           m_Name;
        std::filesystem::path m_AssetPath;

        // Note: Enabled - flag: is texture has been loaded

        struct BaseMeshMaterial
        {
            bool                      Enabled = false;
            Memory::Shared<Texture2D> Texture;
        };

        struct
        {
            BaseMeshMaterial Material;
            glm::vec3        AlbedoColor;
        } MaterialDiffuseData;

        struct
        {
            BaseMeshMaterial Material;
        } MaterialNormalData;

        struct
        {
            BaseMeshMaterial Material;
            float            Roughness;
        } MaterialRoughnessData;

        struct
        {
            BaseMeshMaterial Material;
            float            Metalness;
        } MaterialMetalnessData;

    private:
        friend class Rendering;
    };

    class StaticMesh : public Mesh
    {
    public:
    };

    class AnimatedMesh : public Mesh
    {
    public:
    };
} // namespace Radiant