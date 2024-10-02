
#include <Radiant/Rendering/Mesh.hpp>
#include <Radiant/Rendering/Rendering.hpp>

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>

#include <Radiant/Rendering/Animation/AssimpExporter.hpp>

#include <Radiant/Core/Math/Matrix.hpp>

namespace Radiant
{

#define MESH_DEBUG_LOG 1
#if MESH_DEBUG_LOG
#define MESH_LOG( ... ) RA_TRACE( __VA_ARGS__ )
#else
#define MESH_LOG( ... )
#endif

    struct LogStream : public Assimp::LogStream // TOOD: move to new cpp file
    {
        static void Initialize()
        {
            if ( Assimp::DefaultLogger::isNullLogger() )
            {
                Assimp::DefaultLogger::create( "", Assimp::Logger::VERBOSE );
                Assimp::DefaultLogger::get()->attachStream( new LogStream,
                                                            Assimp::Logger::Err | Assimp::Logger::Warn );
            }
        }

        void write( const char* message ) override
        {
            RA_ERROR( "Assimp error: {0}", message );
        }
    };

    static constexpr unsigned int s_ImportFlags =
         aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_GenNormals |
         aiProcess_GenUVCoords | aiProcess_OptimizeMeshes | aiProcess_ValidateDataStructure;

    namespace
    {
        glm::vec3 GetAlbedoColor( const aiMaterial* material )
        {
            aiColor3D aiColor;
            if ( material->Get( AI_MATKEY_COLOR_DIFFUSE, aiColor ) == aiReturn_SUCCESS )
            {
                return { aiColor.r, aiColor.g, aiColor.b };
            }

            return { 0.0, 0.0, 0.0 };
        }

        float GetRoughness( const aiMaterial* material )
        {
            aiColor3D aiColor;
            float     shininess;
            if ( material->Get( AI_MATKEY_SHININESS, shininess ) != aiReturn_SUCCESS )
            {
                shininess = 80.0f; // Default value
            }

            return 1.0f - glm::sqrt( shininess / 100.0f );
        }

        float GetMetalness( const aiMaterial* material )
        {
            aiColor3D aiColor;
            float     metalness;
            if ( material->Get( AI_MATKEY_REFLECTIVITY, metalness ) != aiReturn_SUCCESS )
            {
                metalness = 0.0f;
            }

            return metalness;
        }

        Common::Memory::Shared<Texture2D> GetDiffuseImage( const std::filesystem::path& pathAsset,
                                                           const aiMaterial*            material )
        {
            aiString texturePath;
            if ( material->GetTexture( aiTextureType_DIFFUSE, 0, &texturePath ) == AI_SUCCESS )
            {
                std::filesystem::path imagePath = Common::Utils::FileSystem::GetFileDirectory( pathAsset ) /
                                                  std::filesystem::path( texturePath.C_Str() );

                MESH_LOG( "aiTextureType_DIFFUSE: {}", imagePath.string() );
                return Texture2D::Create( imagePath );
            }

            return nullptr;
        }

        Common::Memory::Shared<Texture2D> GetNormalsImage( const std::filesystem::path& pathAsset,
                                                           const aiMaterial*            material )
        {
            aiString texturePath;
            if ( material->GetTexture( aiTextureType_NORMALS, 0, &texturePath ) == AI_SUCCESS )
            {
                std::filesystem::path imagePath = Common::Utils::FileSystem::GetFileDirectory( pathAsset ) /
                                                  std::filesystem::path( texturePath.C_Str() );

                MESH_LOG( "aiTextureType_NORMALS: {}", imagePath.string() );
                return Texture2D::Create( imagePath );
            }

            return nullptr;
        }

        Common::Memory::Shared<Texture2D> GetShininessImage( const std::filesystem::path& pathAsset,
                                                             const aiMaterial*            material )
        {
            aiString texturePath;
            if ( material->GetTexture( aiTextureType_SHININESS, 0, &texturePath ) == AI_SUCCESS )
            {
                std::filesystem::path imagePath = Common::Utils::FileSystem::GetFileDirectory( pathAsset ) /
                                                  std::filesystem::path( texturePath.C_Str() );

                MESH_LOG( "aiTextureType_SHININESS: {}", imagePath.string() );
                return Texture2D::Create( imagePath );
            }

            return nullptr;
        }

        Common::Memory::Shared<Texture2D> GetMetalnessImage( const std::filesystem::path& pathAsset,
                                                             const aiMaterial*            material )
        {
            aiString texturePath;
            if ( material->Get( "$raw.ReflectionFactor|file", aiPTI_String, 0, texturePath ) == AI_SUCCESS )
            {
                std::filesystem::path imagePath = Common::Utils::FileSystem::GetFileDirectory( pathAsset ) /
                                                  std::filesystem::path( texturePath.C_Str() );

                MESH_LOG( "$raw.ReflectionFactor|file: {}", imagePath.string() );
                return Texture2D::Create( imagePath );
            }

            return nullptr;
        }

        void SetMaterialImage( const std::string& uniformNameToggle, const std::string& uniformNameImage,
                               const Common::Memory::Shared<Texture2D>& image,
                               const Common::Memory::Shared<Material>&  material )
        {
            if ( image.Raw() != nullptr )
            {
                material->SetBool( uniformNameToggle, true );
                ImageDescriptor desc;
                desc.Name = uniformNameImage;
                material->SetImage2D( desc, image );
            }
            else
            {
                material->SetBool( uniformNameToggle, false );
            }
        }
    } // namespace

    Mesh::Mesh( const std::filesystem::path& filepath ) : m_AssetPath( filepath )
    {
        LogStream::Initialize();

        RADIANT_VERIFY( Common::Utils::FileSystem::Exists( filepath ) );
        RA_TRACE( "Loading mesh: {1}", filepath.string().c_str() );
        m_Name = Common::Utils::FileSystem::GetFileName( filepath );

        m_Importer           = std::make_shared<Assimp::Importer>();
        const aiScene* scene = m_Importer->ReadFile( filepath.string(), s_ImportFlags );
        m_Scene              = scene;

        m_MeshShader = scene->mAnimations != nullptr
                            ? Rendering::GetShaderLibrary()->Get( "AnimPBR_Radiant.glsl" )
                            : Rendering::GetShaderLibrary()->Get( "StaticPBR_Radiant.glsl" );
        m_Material   = Material::Create( m_MeshShader );

        m_Submeshes.reserve( scene->mNumMeshes );

        uint32_t vertexCount = 0;
        uint32_t indexCount  = 0;

        m_GlobalInverseTransform =
             glm::inverse( Math::Matrix::AssimpAIMat4toGLMMat4( scene->mRootNode->mTransformation ) );

        for ( size_t m = 0; m < scene->mNumMeshes; m++ )
        {
            aiMesh* mesh = scene->mMeshes[m];

            Submesh& submesh      = m_Submeshes.emplace_back();
            submesh.BaseVertex    = vertexCount;
            submesh.BaseIndex     = indexCount;
            submesh.MaterialIndex = mesh->mMaterialIndex;
            submesh.IndexCount    = mesh->mNumFaces * 3;

            vertexCount += mesh->mNumVertices;
            indexCount += submesh.IndexCount;

            RADIANT_VERIFY( mesh->HasPositions(), "Meshes require positions." );
            RADIANT_VERIFY( mesh->HasNormals(), "Meshes require normals." );

            auto& aabb = submesh.BoundingBox;
            aabb.Min   = { FLT_MAX, FLT_MAX, FLT_MAX };
            aabb.Max   = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

            for ( int i = 0; i < mesh->mNumVertices; i++ )
            {
                StaticVertex vertex;
                vertex.Position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
                vertex.Normals  = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };

                aabb.Min.x = glm::min( vertex.Position.x, aabb.Min.x );
                aabb.Min.y = glm::min( vertex.Position.y, aabb.Min.y );
                aabb.Min.z = glm::min( vertex.Position.z, aabb.Min.z );
                aabb.Max.x = glm::max( vertex.Position.x, aabb.Max.x );
                aabb.Max.y = glm::max( vertex.Position.y, aabb.Max.y );
                aabb.Max.z = glm::max( vertex.Position.z, aabb.Max.z );

                if ( mesh->HasTangentsAndBitangents() )
                {
                    vertex.Tangent   = { mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z };
                    vertex.Bitangent = { mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z };
                }

                if ( mesh->HasTextureCoords( 0 ) )
                {
                    vertex.TexCoords = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
                }

                m_StaticVertices.push_back( vertex );
            }

            for ( int i = 0; i < mesh->mNumFaces; i++ )
            {
                RADIANT_VERIFY( mesh->mFaces[i].mNumIndices == 3, "Must have 3 indices." );
                Index index;
                index = { mesh->mFaces[i].mIndices[0], mesh->mFaces[i].mIndices[1], mesh->mFaces[i].mIndices[2] };

                m_Indices.push_back( index );
            }

            m_IndexBuffer = IndexBuffer::Create( m_Indices.data(), m_Indices.size() * sizeof( Index ) );

            RADIANT_VERIFY( scene->HasMaterials() );
            if ( scene->HasMaterials() )
            {
                MESH_LOG( "=====================================", filepath.string() );
                MESH_LOG( "====== Materials - {0} ======", filepath.string() );
                MESH_LOG( "=====================================", filepath.string() );

                for ( unsigned int i = 0; i < scene->mNumMaterials; i++ )
                {
                    const aiMaterial* aiMaterial = scene->mMaterials[i];
                    aiString          texturePath;

                    glm::vec3 albedoColor = GetAlbedoColor( aiMaterial );
                    float     metalness   = GetMetalness( aiMaterial );
                    float     roughness   = GetRoughness( aiMaterial );

                    const auto diffuseImage   = GetDiffuseImage( m_AssetPath, aiMaterial );
                    const auto normalsImage   = GetNormalsImage( m_AssetPath, aiMaterial );
                    const auto shininessImage = GetShininessImage( m_AssetPath, aiMaterial );
                    const auto metalnessImage = GetMetalnessImage( m_AssetPath, aiMaterial );

                    m_Material->SetVec3( "u_AlbedoColor", albedoColor );
                    m_Material->SetFloat( "u_Metalness", metalness );
                    m_Material->SetFloat( "u_Roughness", roughness );

                    SetMaterialImage( "u_UseAlbedoTexture", "u_AlbedoTexture", diffuseImage, m_Material );
                    SetMaterialImage( "u_UseNormalTexture", "u_NormalTexture", normalsImage, m_Material );
                    SetMaterialImage( "u_UseMetalnessTexture", "u_MetalnessTexture", metalnessImage, m_Material );
                    SetMaterialImage( "u_UseRoughnessTexture", "u_RoughnessTexture", shininessImage, m_Material );
                }
            }
        }

        TraverseNodes( scene->mRootNode );
    }

    void Mesh::TraverseNodes( aiNode* node, const glm::mat4& parentTransform, uint32_t level )
    {
        glm::mat4 transform = parentTransform * Math::Matrix::AssimpAIMat4toGLMMat4( node->mTransformation );
        for ( uint32_t i = 0; i < node->mNumMeshes; i++ )
        {
            uint32_t mesh     = node->mMeshes[i];
            auto&    submesh  = m_Submeshes[mesh];
            submesh.NodeName  = node->mName.C_Str();
            submesh.Transform = transform;
        }

        for ( uint32_t i = 0; i < node->mNumChildren; i++ )
            TraverseNodes( node->mChildren[i], transform, level + 1 );
    }

    //************************ AnimatedMesh **************************//

    AnimatedMesh::AnimatedMesh( const std::filesystem::path& filepath ) : Mesh( filepath )
    {
        for ( size_t m = 0; m < m_Scene->mNumMeshes; m++ )
        {
            aiMesh*                     mesh = m_Scene->mMeshes[m];
            std::vector<AnimatedVertex> animatedVertices;

            for ( int i = 0; i < mesh->mNumVertices; i++ )
            {

                // NOTE: Actually it is worth to optimize it somehow, at the moment in order not to create several
                // times
                //  the same StaticVertex, we declared it globally, which is also used in StaticMesh, and here we
                //  just throw in AnimatedVertex

                AnimatedVertex vertex;
                vertex.StaticVertexData = m_StaticVertices[i];
                animatedVertices.push_back( vertex );
            }

            ExtractBoneWeightForVertices( animatedVertices, mesh, m_Scene );
            m_VertexBuffer = VertexBuffer::Create( animatedVertices.data(),
                                                   animatedVertices.size() * sizeof( AnimatedVertex ) );
        }
        BuildBonesHierarchy( m_Scene->mRootNode );
    }

    void AnimatedMesh::ExtractBoneWeightForVertices( std::vector<AnimatedVertex>& vertices, aiMesh* mesh,
                                                     const aiScene* scene )
    {
        for ( int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex )
        {
            int         boneID   = -1;
            std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
            if ( m_BoneInfo.find( boneName ) == m_BoneInfo.end() )
            {
                Animation::BoneInfo newBoneInfo;
                newBoneInfo.ID = m_BoneInfo.size();
                newBoneInfo.BoneOffset =
                     Math::Matrix::AssimpAIMat4toGLMMat4( mesh->mBones[boneIndex]->mOffsetMatrix );
                m_BoneInfo[boneName] = newBoneInfo;
                boneID               = newBoneInfo.ID;
            }
            else
            {
                boneID = m_BoneInfo[boneName].ID;
            }
            RADIANT_VERIFY( boneID != -1 );
            auto weights    = mesh->mBones[boneIndex]->mWeights;
            int  numWeights = mesh->mBones[boneIndex]->mNumWeights;

            for ( int weightIndex = 0; weightIndex < numWeights; ++weightIndex )
            {
                int   vertexId = weights[weightIndex].mVertexId;
                float weight   = weights[weightIndex].mWeight;
                RADIANT_VERIFY( vertexId <= vertices.size() );
                vertices[vertexId].BoneInfluenceData.AddBoneData( boneID, weight );
            }
        }

        auto exporter = Animation::Exporter();
        m_Skeleton    = exporter.ImportSkeleton( m_AssetPath.string() ).value();
        m_Animations.push_back( exporter.ImportAnimation( m_AssetPath.string(), m_Skeleton ).value() );

        m_AnimationController =
             std::make_unique<Animation::AnimationController>( m_Animations.back(), m_Skeleton );
    }

    void AnimatedMesh::BuildBonesHierarchy( const aiNode* node, std::optional<uint32_t> parentIndex )
    {
        m_BonesHierarchy_RAW.push_back( { node->mName.C_Str(), parentIndex } );

        uint32_t currentIndex = m_BonesHierarchy_RAW.size() - 1;

        for ( uint32_t i = 0; i < node->mNumChildren; ++i )
        {
            BuildBonesHierarchy( node->mChildren[i], currentIndex );
        }
    }

    //****************************************************//

    //************************ StaticMesh **************************//

    StaticMesh::StaticMesh( const std::filesystem::path& filepath ) : Mesh( filepath )
    {
        m_VertexBuffer =
             VertexBuffer::Create( m_StaticVertices.data(), m_StaticVertices.size() * sizeof( StaticVertex ) );
    }

} // namespace Radiant