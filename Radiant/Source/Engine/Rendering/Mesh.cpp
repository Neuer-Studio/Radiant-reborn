
#include <Radiant/Rendering/Mesh.hpp>

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
    glm::mat4 Mat4FromAssimpMat4( const aiMatrix4x4& matrix )
    {
        glm::mat4 result;
        // the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
        result[0][0] = matrix.a1;
        result[1][0] = matrix.a2;
        result[2][0] = matrix.a3;
        result[3][0] = matrix.a4;
        result[0][1] = matrix.b1;
        result[1][1] = matrix.b2;
        result[2][1] = matrix.b3;
        result[3][1] = matrix.b4;
        result[0][2] = matrix.c1;
        result[1][2] = matrix.c2;
        result[2][2] = matrix.c3;
        result[3][2] = matrix.c4;
        result[0][3] = matrix.d1;
        result[1][3] = matrix.d2;
        result[2][3] = matrix.d3;
        result[3][3] = matrix.d4;
        return result;
    }

    glm::mat4 CalculateGlobalInverseTransform( const aiNode* rootNode )
    {
        glm::mat4     globalTransform = Mat4FromAssimpMat4( rootNode->mTransformation );
        const aiNode* parent          = rootNode->mParent;

        while ( parent )
        {
            globalTransform = Mat4FromAssimpMat4( parent->mTransformation ) * globalTransform;
            parent          = parent->mParent;
        }

        return glm::inverse( globalTransform );
    }

    struct LogStream : public Assimp::LogStream
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

    Mesh::Mesh( const std::filesystem::path& filepath )
    {
        LogStream::Initialize();
        RADIANT_VERIFY( Utils::FileSystem::Exists( filepath ) );
        RA_TRACE( "Loading mesh: {0}", filepath.string().c_str() );

        m_Name = Utils::FileSystem::GetFileName( filepath );

        const auto s_Importer = std::make_unique<Assimp::Importer>();

        const aiScene* scene = s_Importer->ReadFile( filepath.string(), s_ImportFlags );
        m_Submeshes.reserve( scene->mNumMeshes );

        uint32_t vertexCount = 0;
        uint32_t indexCount  = 0;

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

            std::vector<StaticVertex>   staticVertices;
            std::vector<AnimatedVertex> animatedVertices;

            for ( int i = 0; i < mesh->mNumVertices; i++ )
            {
                AnimatedVertex vertex;
                vertex.StaticVertexData.Position = { mesh->mVertices[i].x, mesh->mVertices[i].y,
                                                     mesh->mVertices[i].z };
                vertex.StaticVertexData.Normals  = { mesh->mNormals[i].x, mesh->mNormals[i].y,
                                                     mesh->mNormals[i].z };

                /*aabb.Min.x = glm::min(vertex.Position.x, aabb.Min.x);
                aabb.Min.y = glm::min(vertex.Position.y, aabb.Min.y);
                aabb.Min.z = glm::min(vertex.Position.z, aabb.Min.z);
                aabb.Max.x = glm::max(vertex.Position.x, aabb.Max.x);
                aabb.Max.y = glm::max(vertex.Position.y, aabb.Max.y);
                aabb.Max.z = glm::max(vertex.Position.z, aabb.Max.z);*/

                if ( mesh->HasTangentsAndBitangents() )
                {
                    vertex.StaticVertexData.Tangent   = { mesh->mTangents[i].x, mesh->mTangents[i].y,
                                                          mesh->mTangents[i].z };
                    vertex.StaticVertexData.Bitangent = { mesh->mBitangents[i].x, mesh->mBitangents[i].y,
                                                          mesh->mBitangents[i].z };
                }

                if ( mesh->HasTextureCoords( 0 ) )
                {
                    vertex.StaticVertexData.TexCoords = { mesh->mTextureCoords[0][i].x,
                                                          mesh->mTextureCoords[0][i].y };
                }

                animatedVertices.push_back( vertex );
            }

            ExtractBoneWeightForVertices( animatedVertices, mesh, scene );

            if ( scene->mNumAnimations )
            {
                Animation::Exporter exporter;

                const auto names = exporter.GetAnimationNames( scene );
                m_Joints = exporter.ImportJoints(scene).value();

                m_Animations.emplace_back( exporter.ImportAnimation( scene, names[0], m_Joints).value() );

                m_AnimationController =
                    std::make_unique<Animation::AnimationController>(m_Animations[0], m_Joints, m_BoneInfo);

                m_AnimationController->ffff(glm::inverse(Mat4FromAssimpMat4(scene->mRootNode->mTransformation)));
            }

            m_VertexBuffer =
                 VertexBuffer::Create( animatedVertices.data(), animatedVertices.size() * sizeof(AnimatedVertex) );

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
                    aiColor3D         aiColor;

                    MaterialDiffuseData.AlbedoColor = { 0.0, 0.0, 0.0 };

                    if ( aiMaterial->Get( AI_MATKEY_COLOR_DIFFUSE, aiColor ) == aiReturn_SUCCESS )
                        MaterialDiffuseData.AlbedoColor = { aiColor.r, aiColor.g, aiColor.b };
                    float shininess, metalness;
                    if ( aiMaterial->Get( AI_MATKEY_SHININESS, shininess ) != aiReturn_SUCCESS )
                        shininess = 80.0f; // Default value

                    if ( aiMaterial->Get( AI_MATKEY_REFLECTIVITY, metalness ) != aiReturn_SUCCESS )
                        metalness = 0.0f;
                    float roughness = 1.0f - glm::sqrt( shininess / 100.0f );

                    if ( aiMaterial->GetTexture( aiTextureType_DIFFUSE, 0, &texturePath ) ==
                         AI_SUCCESS ) // TODO: is texture loaded -> set a texture to material, or just set a vec3
                                      // data
                    {
                        std::filesystem::path imagePath = Utils::FileSystem::GetFileDirectory( filepath ) /
                                                          std::filesystem::path( texturePath.C_Str() );

                        MaterialDiffuseData.Material.Enabled = true;
                        MaterialDiffuseData.Material.Texture = Texture2D::Create( imagePath );

                        MESH_LOG( "aiTextureType_DIFFUSE: {}", imagePath.string() );
                    }

                    if ( aiMaterial->GetTexture( aiTextureType_NORMALS, 0, &texturePath ) == AI_SUCCESS )
                    {
                        std::filesystem::path imagePath = Utils::FileSystem::GetFileDirectory( filepath ) /
                                                          std::filesystem::path( texturePath.C_Str() );

                        MaterialDiffuseData.Material.Enabled = true;
                        MaterialDiffuseData.Material.Texture = Texture2D::Create( imagePath );

                        MESH_LOG( "aiTextureType_NORMALS: {}", imagePath.string() );
                    }

                    MaterialRoughnessData.Roughness = roughness;
                    if ( aiMaterial->GetTexture( aiTextureType_SHININESS, 0, &texturePath ) == AI_SUCCESS )
                    {
                        std::filesystem::path imagePath = Utils::FileSystem::GetFileDirectory( filepath ) /
                                                          std::filesystem::path( texturePath.C_Str() );

                        MaterialDiffuseData.Material.Enabled = true;
                        MaterialDiffuseData.Material.Texture = Texture2D::Create( imagePath );

                        MESH_LOG( "aiTextureType_SHININESS: {}", imagePath.string() );
                    }

                    MaterialMetalnessData.Metalness = metalness;
                    if ( aiMaterial->Get( "$raw.ReflectionFactor|file", aiPTI_String, 0, texturePath ) ==
                         AI_SUCCESS )
                    {
                        std::filesystem::path imagePath = Utils::FileSystem::GetFileDirectory( filepath ) /
                                                          std::filesystem::path( texturePath.C_Str() );

                        MaterialDiffuseData.Material.Enabled = true;
                        MaterialDiffuseData.Material.Texture = Texture2D::Create( imagePath );

                        MESH_LOG( "aiTextureType_SHININESS: {}", imagePath.string() );
                    }
                }
            }
        }

        TraverseNodes( scene->mRootNode );
    }

    void Mesh::Use() const
    {
        m_VertexBuffer->Use();
        m_IndexBuffer->Use();
    }

    void Mesh::ExtractBoneWeightForVertices( std::vector<AnimatedVertex>& vertices, aiMesh* mesh,
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
                boneID                  = newBoneInfo.ID;
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
                vertices[vertexId].AddBoneData( boneID, weight );
            }
        }
    }

    void Mesh::TraverseNodes( aiNode* node, const glm::mat4& parentTransform, uint32_t level )
    {
        glm::mat4 transform = parentTransform * Mat4FromAssimpMat4( node->mTransformation );
        for ( uint32_t i = 0; i < node->mNumMeshes; i++ )
        {
            uint32_t mesh    = node->mMeshes[i];
            auto&    submesh = m_Submeshes[mesh];
            // submesh.NodeName = node->mName.C_Str();
            submesh.Transform = transform;
        }

        // HZ_MESH_LOG("{0} {1}", LevelToSpaces(level), node->mName.C_Str());

        for ( uint32_t i = 0; i < node->mNumChildren; i++ )
            TraverseNodes( node->mChildren[i], transform, level + 1 );
    }
} // namespace Radiant