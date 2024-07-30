#pragma once

#include <Radiant/Rendering/Animation/Animation.hpp>
#include <Radiant/Rendering/Animation/Joint.hpp>
#include <Radiant/Core/Timestep.hpp>

#include <Radiant/Rendering/Animation/BoneInfo.hpp>

namespace Radiant::Animation
{

    class AnimationController
    {
   public:
        AnimationController( const class Animation& animation, const Joints& joints,
                             std::unordered_map<std::string, BoneInfo> map );

        void SetAnimation( const class Animation& animation );

        const auto& GetFinalBonesTransform() const { return m_FinalBoneMatrices; }
        void ffff( glm::mat4 sdf )
        {
            m_sdf = sdf;
        }

        void UpdateAnimation( Timestep ts );
        void CalculateBoneTransform(uint32_t jointID, const glm::mat4& parentTransform);

        const auto sdfsdf() const
        {
            return m_FinalBoneMatrices;
        }
        std::unordered_map<std::string, BoneInfo> mmap;

   private:
        std::optional<glm::mat4> UpdateTransforms( float animationTime, uint32_t jointID );

   private:
        Animation m_Animation;
        Joints    m_Joints;
        float     m_CurrentTime = 0.0f; // current animation time
   private:
        template <typename T>
        std::optional<uint32_t> FindIndex( const std::vector<T>& keys, float animationTime, uint32_t jointID )
        {
           /* auto it = std::find_if( keys.begin(), keys.end() - 1,
                                    [&]( const auto& key, const auto& nextKey )
                                    { return animationTime < nextKey.FrameTime && nextKey.Track == jointID; } );

            if ( it != keys.end() - 1 )
            {
                return std::distance( keys.begin(), it );
            }*/

            for (uint32_t i = 0; i < keys.size() - 1; i++)
            {
                if ( animationTime < keys[i + 1].FrameTime && keys[i + 1].Track == jointID)
                    return i;
            }

            return std::nullopt;
        }

        template <typename T, typename T2>
        std::optional<std::pair<T2, T2>> FindIndexAndGetPair( const std::vector<T>& keys, float animationTime,
                                                              uint32_t jointID )
        {
            const auto& indexFind = FindIndex( keys, animationTime, jointID );
            if ( !indexFind )
                return std::nullopt;

            uint32_t p0Index = indexFind.value();

            const auto& p0Position = keys[p0Index];
            const auto& p1Position = keys[p0Index + 1];

            return std::make_pair( p0Position, p1Position );
        }

        std::optional<glm::mat4> InterpolatePosition( float animationTime, uint32_t jointID );
        std::optional<glm::mat4> InterpolateRotation( float animationTime, uint32_t jointID );
        std::optional<glm::mat4> InterpolateScaling( float animationTime, uint32_t jointID );

        [[nodiscard]] glm::mat4 GetParrentTransform(uint32_t jointID);

   private:
       [[nodiscard]] bool IsAnimationFinished();
       uint32_t m_CurrentAnimnationFrame = 0;
   private:
        std::vector<glm::mat4> m_FinalBoneMatrices;
        std::vector<glm::mat4> m_ParentFinalBoneMatrices;
        glm::mat4              m_sdf;
    };
} // namespace Radiant::Animation