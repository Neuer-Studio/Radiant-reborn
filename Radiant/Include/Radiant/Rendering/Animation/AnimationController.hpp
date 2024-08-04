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
        AnimationController( const class Animation& animation, const Joints& joints );

        void                           SetAnimation( const class Animation& animation );
        const std::optional<glm::mat4> GetBoneUpdateTransform( uint32_t boneID );

        void OnUpdate( Timestep ts );

        auto& GetJoints() const
        {
            return m_Joints;
        }

        float GetPlaybackSpeed() const
        {
            return m_PlaybackSpeed;
        }
        void SetPlaybackSpeed( const float f )
        {
            m_PlaybackSpeed = f;
        }
    private:
        std::optional<glm::mat4> UpdateTransforms( float animationTime, uint32_t jointID );

    private:
        float m_PlaybackSpeed = 1.0;

        bool      m_IsAnimationPlaying = true;
        Animation m_Animation;
        Joints    m_Joints;
        float     m_AnimationTime = 0.0f; // current animation time
    private:
        template <typename T>
        [[nodiscard]] std::vector<T> FilterAndSortByTrack( const std::vector<T>& keys, uint32_t track )
        {
            std::vector<T> filteredKeys;

            std::copy_if( keys.begin(), keys.end(), std::back_inserter( filteredKeys ),
                          [track]( const T& key ) { return key.Track == track; } );

            std::sort( filteredKeys.begin(), filteredKeys.end(),
                       []( const T& a, const T& b ) { return a.FrameTime < b.FrameTime; } );

            return filteredKeys;
        }

        template <typename T>
        std::pair<std::vector<T>, std::optional<uint32_t>>
        FindIndex( const std::vector<T>& keys, float animationTime, uint32_t jointID, uint32_t start = 0 )
        {
            const auto& sortedKeys = FilterAndSortByTrack( keys, jointID );

            for ( uint32_t i = 0; i < sortedKeys.size() - 1; i++ )
            {
                if ( animationTime < sortedKeys[i + 1].FrameTime )
                    return { sortedKeys, i };
            }

            return { sortedKeys, std::nullopt };
        }

        template <typename T, typename T2>
        std::optional<std::pair<T2, T2>> FindIndexAndGetPair( const std::vector<T>& keys, float animationTime,
                                                              uint32_t jointID )
        {
            const auto& indexFind  = FindIndex( keys, animationTime, jointID );
            const auto& index      = indexFind.second;
            const auto& sortedKeys = indexFind.first;

            if ( !index )
                return std::nullopt;

            uint32_t p0Index = index.value();

            const auto& p0Position = sortedKeys[p0Index];
            const auto& p1Position = sortedKeys[p0Index + 1];

            return std::make_pair( p0Position, p1Position );
        }

        std::optional<glm::mat4> InterpolatePosition( float animationTime, uint32_t jointID );
        std::optional<glm::mat4> InterpolateRotation( float animationTime, uint32_t jointID );
        std::optional<glm::mat4> InterpolateScaling( float animationTime, uint32_t jointID );
    };
} // namespace Radiant::Animation