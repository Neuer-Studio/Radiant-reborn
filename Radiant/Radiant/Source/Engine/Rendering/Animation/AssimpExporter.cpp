#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>

#include <Radiant/Core/Math/Matrix.hpp>

#include <Radiant/Rendering/Animation/AssimpExporter.hpp>
#include <map>
#include <set>

namespace Radiant::Animation
{
    static const uint32_t s_AnimationImportFlags =
         aiProcess_CalcTangentSpace | // Create binormals/tangents just in case
         aiProcess_Triangulate |      // Make sure we're triangles
         aiProcess_SortByPType |      // Split meshes by primitive type
         aiProcess_GenNormals |       // Make sure we have legit normals
         aiProcess_GenUVCoords |      // Convert UVs if required
         //		aiProcess_OptimizeGraph |
         aiProcess_OptimizeMeshes | // Batch draws where possible
         aiProcess_JoinIdenticalVertices |
         aiProcess_LimitBoneWeights | // If more than N (=4) bone weights, discard least influencing bones and
                                      // renormalise sum to 1
         aiProcess_GlobalScale |      // e.g. convert cm to m for fbx import (and other formats where cm is native)
         //		aiProcess_PopulateArmatureData |    // not currently using this data
         aiProcess_ValidateDataStructure; // Validation 

    class BoneHierarchy
    {
    public:
        BoneHierarchy( const aiScene* scene );

        void                  ExtractBones();
        void                  TraverseNode( aiNode* node, Skeleton* skeleton);
        void                  TraverseBone( aiNode* node, Skeleton* skeleton, std::optional<uint32_t> parentIndex );
        std::optional<Skeleton> CreateSkeleton();

    private:
        std::set<std::string_view> m_Bones;
        const aiScene*             m_Scene;
    };

    std::vector<std::string> Exporter::GetAnimationNames( const aiScene* scene ) const
    {
        std::vector<std::string> animationNames;
        if ( scene )
        {
            animationNames.reserve( scene->mNumAnimations );
            for ( size_t i = 0; i < scene->mNumAnimations; ++i )
            {
                animationNames.emplace_back( scene->mAnimations[i]->mName.C_Str() );
            }
        }
        return animationNames;
    }

    template <typename T>
    struct KeyFrame
    {
        float FrameTime;
        T     Value;
        KeyFrame( const float frameTime, const T& value ) : FrameTime( frameTime ), Value( value )
        {
        }
    };

    struct Channel
    {
        std::vector<KeyFrame<glm::vec3>> Translations;
        std::vector<KeyFrame<glm::quat>> Rotations;
        std::vector<KeyFrame<glm::vec3>> Scales;
        uint32_t                         Index;
    };

    // Import all of the channels from anim that refer to bones in skeleton
    static auto ImportChannels( const aiAnimation* anim, const Skeleton& skeleton)
    {
        std::vector<Channel> channels;

        std::unordered_map<std::string_view, uint32_t> boneIndices;
        for ( uint32_t i = 0; i < skeleton.BoneCount(); ++i )
        {
            boneIndices.emplace(skeleton.GetBoneName( i ), i );
        }

        std::set<std::tuple<uint32_t, aiNodeAnim*>> validChannels;
        for ( uint32_t channelIndex = 0; channelIndex < anim->mNumChannels; ++channelIndex )
        {
            aiNodeAnim* nodeAnim = anim->mChannels[channelIndex];
            auto        it       = boneIndices.find( nodeAnim->mNodeName.C_Str() );
            if ( it != boneIndices.end() )
            {
                validChannels.emplace( it->second, nodeAnim );
            }
        }

        channels.resize(skeleton.BoneCount() );
        for ( auto [boneIndex, nodeAnim] : validChannels )
        {
            channels[boneIndex].Index = boneIndex;
            channels[boneIndex].Translations.reserve( nodeAnim->mNumPositionKeys + 1 );
            channels[boneIndex].Rotations.reserve( nodeAnim->mNumRotationKeys + 1 );
            channels[boneIndex].Scales.reserve( nodeAnim->mNumScalingKeys + 1 );

            // Note: There is no need to check for duplicate keys (i.e. multiple keys all at same frame time)
            //       because Assimp throws these out for us
            for ( uint32_t keyIndex = 0; keyIndex < nodeAnim->mNumPositionKeys; ++keyIndex )
            {
                aiVectorKey key       = nodeAnim->mPositionKeys[keyIndex];
                float frameTime = std::clamp(static_cast<float>(key.mTime / anim->mDuration), 0.0f, 1.0f);

                channels[boneIndex].Translations.emplace_back(
                     frameTime, glm::vec3{ static_cast<float>( key.mValue.x ), static_cast<float>( key.mValue.y ),
                                           static_cast<float>( key.mValue.z ) } );
            }
            for ( uint32_t keyIndex = 0; keyIndex < nodeAnim->mNumRotationKeys; ++keyIndex )
            {
                aiQuatKey key       = nodeAnim->mRotationKeys[keyIndex];
                float frameTime = std::clamp(static_cast<float>(key.mTime / anim->mDuration), 0.0f, 1.0f);

                channels[boneIndex].Rotations.emplace_back(
                     frameTime,
                     glm::quat{ static_cast<float>( key.mValue.w ), static_cast<float>( key.mValue.x ),
                                static_cast<float>( key.mValue.y ), static_cast<float>( key.mValue.z ) } );
            }
            for ( uint32_t keyIndex = 0; keyIndex < nodeAnim->mNumScalingKeys; ++keyIndex )
            {
                aiVectorKey key       = nodeAnim->mScalingKeys[keyIndex];
                float frameTime = std::clamp(static_cast<float>(key.mTime / anim->mDuration), 0.0f, 1.0f);

                channels[boneIndex].Scales.emplace_back(
                     frameTime, glm::vec3{ static_cast<float>( key.mValue.x ), static_cast<float>( key.mValue.y ),
                                           static_cast<float>( key.mValue.z ) } );
            }
        }

        return channels;
    }

    static auto ConcatenateChannelsAndSort( const std::vector<Channel>& channels )
    {
        // We concatenate the translations for all the channels into one big long vector, and then sort
        // it on _previous_ frame time.  This gives us an efficient way to sample the key frames later on.
        // (taking advantage of fact that animation almost always plays forwards)

        uint32_t numTranslations = 0;
        uint32_t numRotations    = 0;
        uint32_t numScales       = 0;

        for ( auto channel : channels )
        {
            numTranslations += static_cast<uint32_t>( channel.Translations.size() );
            numRotations += static_cast<uint32_t>( channel.Rotations.size() );
            numScales += static_cast<uint32_t>( channel.Scales.size() );
        }

        std::vector<std::pair<float, TranslationKey>> translationKeysTemp;
        std::vector<std::pair<float, RotationKey>>    rotationKeysTemp;
        std::vector<std::pair<float, ScaleKey>>       scaleKeysTemp;
        translationKeysTemp.reserve( numTranslations );
        rotationKeysTemp.reserve( numRotations );
        scaleKeysTemp.reserve( numScales );
        for ( const auto& channel : channels )
        {
            float prevFrameTime = -1.0f;
            for ( const auto& translation : channel.Translations )
            {
                translationKeysTemp.emplace_back(
                     prevFrameTime, TranslationKey{ translation.FrameTime, channel.Index, translation.Value } );
                prevFrameTime = translation.FrameTime;
            }

            prevFrameTime = -1.0f;
            for ( const auto& rotation : channel.Rotations )
            {
                rotationKeysTemp.emplace_back( prevFrameTime,
                                               RotationKey{ rotation.FrameTime, channel.Index, rotation.Value } );
                prevFrameTime = rotation.FrameTime;
            }

            prevFrameTime = -1.0f;
            for ( const auto& scale : channel.Scales )
            {
                scaleKeysTemp.emplace_back( prevFrameTime,
                                            ScaleKey{ scale.FrameTime, channel.Index, scale.Value } );
                prevFrameTime = scale.FrameTime;
            }
        }
        std::sort( translationKeysTemp.begin(), translationKeysTemp.end(),
                   []( const auto& a, const auto& b ) {
                       return ( a.first < b.first ) ||
                              ( ( a.first == b.first ) && a.second.Track < b.second.Track );
                   } );
        std::sort( rotationKeysTemp.begin(), rotationKeysTemp.end(),
                   []( const auto& a, const auto& b ) {
                       return ( a.first < b.first ) ||
                              ( ( a.first == b.first ) && a.second.Track < b.second.Track );
                   } );
        std::sort( scaleKeysTemp.begin(), scaleKeysTemp.end(),
                   []( const auto& a, const auto& b ) {
                       return ( a.first < b.first ) ||
                              ( ( a.first == b.first ) && a.second.Track < b.second.Track );
                   } );

        return std::tuple{ translationKeysTemp, rotationKeysTemp, scaleKeysTemp };
    }

    static auto ExtractKeys( const std::vector<std::pair<float, TranslationKey>>& translationKeysTemp,
                             const std::vector<std::pair<float, RotationKey>>&    rotationKeysTemp,
                             const std::vector<std::pair<float, ScaleKey>>&       scaleKeysTemp )
    {
        std::vector<TranslationKey> translationKeys;
        std::vector<RotationKey>    rotationKeys;
        std::vector<ScaleKey>       scaleKeys;
        translationKeys.reserve( translationKeysTemp.size() );
        rotationKeys.reserve( rotationKeysTemp.size() );
        scaleKeys.reserve( scaleKeysTemp.size() );
        for ( const auto& translation : translationKeysTemp )
        {
            translationKeys.emplace_back( translation.second );
        }
        for ( const auto& rotation : rotationKeysTemp )
        {
            rotationKeys.emplace_back( rotation.second );
        }
        for ( const auto& scale : scaleKeysTemp )
        {
            scaleKeys.emplace_back( scale.second );
        }

        return std::tuple{ translationKeys, rotationKeys, scaleKeys };
    }

    std::optional<Animation> Exporter::ImportAnimation( const aiScene* scene, const std::string_view animationName,
                                                        const Skeleton& skeleton )
    {
        if ( !scene )
        {
            return std::nullopt;
        }

        for ( uint32_t animIndex = 0; animIndex < scene->mNumAnimations; ++animIndex )
        {
            const aiAnimation* anim = scene->mAnimations[animIndex];
            if ( animationName == anim->mName.C_Str() )
            {
                auto channels = ImportChannels( anim, skeleton );
                auto [translationKeysTemp, rotationKeysTemp, scaleKeysTemp] =
                     ConcatenateChannelsAndSort( channels );
                auto [translationKeys, rotationKeys, scaleKeys] =
                     ExtractKeys( translationKeysTemp, rotationKeysTemp, scaleKeysTemp );

                double samplingRate = anim->mTicksPerSecond;
                if ( samplingRate < 0.0001 )
                {
                    samplingRate = 1.0;
                }

                Animation animation( animationName, static_cast<float>(anim->mDuration / samplingRate));
                animation.SetKeyFrames( translationKeys, rotationKeys, scaleKeys );
                return animation;
            }
        }
        return std::nullopt;
    }

    std::optional<Animation> Exporter::ImportAnimation( const std::string_view filename,
                                                                 const Skeleton&        skeleton )
    {
        Assimp::Importer importer;
        const aiScene*   scene = importer.ReadFile( filename.data(), s_AnimationImportFlags );
        auto animationNames = GetAnimationNames(scene);
        return ImportAnimation(scene, animationNames.front(), skeleton);
    }

    std::optional<Radiant::Animation::Skeleton> Exporter::ImportSkeleton( const aiScene* scene ) const
    {
        BoneHierarchy boneHierarchy( scene );
        return boneHierarchy.CreateSkeleton();
    }

    std::optional<Radiant::Animation::Skeleton> Exporter::ImportSkeleton( const std::string_view filename )
    {
        Assimp::Importer importer;
        const aiScene*   scene = importer.ReadFile( filename.data(), s_AnimationImportFlags );
        return ImportSkeleton( scene );
    }

    BoneHierarchy::BoneHierarchy( const aiScene* scene ) : m_Scene( scene )
    {
    }

    std::optional<Skeleton> BoneHierarchy::CreateSkeleton()
    {
        if ( !m_Scene )
        {
            return std::nullopt;
        }

        ExtractBones();
        if ( m_Bones.empty() )
        {
            return std::nullopt;
        }

        auto skeleton = Skeleton( static_cast<uint32_t>( m_Bones.size() ) );
        TraverseNode( m_Scene->mRootNode, &skeleton );

        return skeleton;
    }

    void BoneHierarchy::ExtractBones()
    {
        // Note: ASSIMP does not appear to support import of digital content files that contain _only_ an
        // armature/skeleton and no mesh.
        for ( uint32_t meshIndex = 0; meshIndex < m_Scene->mNumMeshes; ++meshIndex )
        {
            const aiMesh* mesh = m_Scene->mMeshes[meshIndex];
            for ( uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex )
            {
                m_Bones.emplace( mesh->mBones[boneIndex]->mName.C_Str() );
            }
        }
    }

    void BoneHierarchy::TraverseNode( aiNode* node, Skeleton* skeleton)
    {
        if ( m_Bones.find( node->mName.C_Str() ) != m_Bones.end() )
        {
            TraverseBone( node, skeleton, std::nullopt );
        }
        else
        {
            for ( uint32_t nodeIndex = 0; nodeIndex < node->mNumChildren; ++nodeIndex )
            {
                TraverseNode( node->mChildren[nodeIndex], skeleton);
            }
        }
    }

    void BoneHierarchy::TraverseBone( aiNode* node, Skeleton* skeleton, std::optional<uint32_t> parentIndex )
    {
        using namespace Math::Matrix;

        BoneInformation info;
        info.BoneName        = node->mName.C_Str();
        info.ParentBoneIndex = parentIndex;

        const auto decMatrix  = DecomposeTransform( AssimpAIMat4toGLMMat4( node->mTransformation ) );
        info.BoneRotation    = decMatrix.Rotation;
        info.BoneScale       = decMatrix.Scale;
        info.BoneTranslation = decMatrix.Translation;

        uint32_t boneIndex = skeleton->AddBone( info );
        for ( uint32_t nodeIndex = 0; nodeIndex < node->mNumChildren; ++nodeIndex )
        {
            TraverseBone( node->mChildren[nodeIndex], skeleton, boneIndex );
        }
    }

} // namespace Radiant::Animation