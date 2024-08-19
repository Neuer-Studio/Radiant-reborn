#pragma once

#include <Radiant/Core/Timestep.hpp>
#include <Radiant/Rendering/Animation/Animation.hpp>
#include <Radiant/Rendering/Animation/BoneInfo.hpp>
#include <Radiant/Rendering/Animation/Skeleton.hpp>

namespace Radiant::Animation
{

    struct RootPose
    {
        glm::vec3 Translation = { 0.0f, 0.0f, 0.0f };       // aka. Position
        glm::quat Rotation    = glm::identity<glm::quat>(); // aka. Orientation
    };
    using RootMotion = RootPose;

    template <typename T>
    class SamplingCache
    {
    public:
        void Resize( const uint32_t numTracks )
        {
            m_Values.resize( numTracks * 2,
                             T() ); // Values vector stores the current and next key for each track, interleaved.
            // These are the values that we interpolate to sample animation at a given time
            m_FrameTimes.resize( numTracks * 2,
                                 0.0f ); // FrameTimes vector stores the frame time for current and next key.  This
            // is used to figure out the interpolation between values.
        }

        void Reset( const Animation* animation, const std::vector<T>& values )
        {
            for ( uint32_t i = 0, N = static_cast<uint32_t>( values.size() ); i < N; ++i )
            {
                m_Values[NextIndex( i )]     = values[i];
                m_FrameTimes[NextIndex( i )] = 0.0f;
            }
            m_Animation = animation;
            m_Cursor    = 0;
        }

        // step cache forward to given time, using given key frames
        void Step( const float sampleTime, const std::vector<AnimationKey<T>>& keys )
        {
            if ( ( m_Cursor == static_cast<uint32_t>( keys.size() ) ) || ( sampleTime < m_PrevSampleTime ) )
            {
                Loop();
            }
            auto track = keys[m_Cursor].Track;
            while ( m_FrameTimes[NextIndex( track )] <= sampleTime )
            {
                m_Values[CurrentIndex( track )]     = m_Values[NextIndex( track )];
                m_Values[NextIndex( track )]        = keys[m_Cursor].Value;
                m_FrameTimes[CurrentIndex( track )] = m_FrameTimes[NextIndex( track )];
                m_FrameTimes[NextIndex( track )]    = keys[m_Cursor].FrameTime;

                if ( ++m_Cursor == static_cast<uint32_t>( keys.size() ) )
                {
                    break;
                }
                track = keys[m_Cursor].Track;
            }
            m_PrevSampleTime = sampleTime;
        }

        // loop back to the beginning of animation
        void Loop()
        {
            m_Cursor = 0;
            for ( uint32_t track = 0, N = static_cast<uint32_t>( m_Values.size() / 2 ); track < N; ++track )
            {
                m_FrameTimes[NextIndex( track )] = 0.0;
            }
            m_PrevSampleTime = 0.0f;
        }
        void Interpolate( const float sampleTime, std::vector<T>& result,
                          const std::function<T( const T&, const T&, const float )>& interpolater )
        {
            for ( uint32_t i = 0, N = static_cast<uint32_t>( m_Values.size() ); i < N; i += 2 )
            {
                const float t = ( sampleTime - m_FrameTimes[i] ) / ( m_FrameTimes[i + 1] - m_FrameTimes[i] );
                RADIANT_VERIFY( t > -0.0000001f && t < 1.0000001f );
                result[i / 2] =
                     interpolater( m_Values[i], m_Values[i + 1],
                                   ( sampleTime - m_FrameTimes[i] ) / ( m_FrameTimes[i + 1] - m_FrameTimes[i] ) );
            }
        }

    public:
        static uint32_t CurrentIndex( const uint32_t i )
        {
            return 2 * i;
        }
        static uint32_t NextIndex( const uint32_t i )
        {
            return 2 * i + 1;
        }

    private:
        std::vector<T>     m_Values;
        std::vector<float> m_FrameTimes;

        float    m_PrevSampleTime = 0.0f;
        uint32_t m_Cursor         = 0;
    };
    using TranslationCache = SamplingCache<glm::vec3>;
    using RotationCache    = SamplingCache<glm::quat>;
    using ScaleCache       = SamplingCache<glm::vec3>;

    class AnimationController
    {
    public:
        AnimationController( const class Animation& animation, const Skeleton& skeleton );

        void SetAnimation( const class Animation& animation );
        void OnUpdate( Timestep ts );

        float GetPlaybackSpeed() const
        {
            return m_PlaybackSpeed;
        }
        void SetPlaybackSpeed( const float f )
        {
            m_PlaybackSpeed = f;
        }
        const RootMotion& GetRootMotion() const
        {
            return m_RootMotion;
        }

        glm::vec3 GetTranslation( uint32_t boneIndex ) const
        {
            return m_LocalTranslations[boneIndex];
        }
        glm::quat GetRotation( uint32_t boneIndex ) const
        {
            return m_LocalRotations[boneIndex];
        }
        glm::vec3 GetScale( uint32_t boneIndex ) const
        {
            return m_LocalScales[boneIndex];
        }

    private:
        void SampleAnimation();

    private:
        TranslationCache m_TranslationCache;
        RotationCache    m_RotationCache;
        ScaleCache       m_ScaleCache;

        float m_PlaybackSpeed = 1.0;

        bool       m_IsAnimationPlaying = true;
        Animation  m_Animation;
        Skeleton   m_Skeleton;
        float      m_PreviousAnimationTime = -FLT_MAX;
        float      m_AnimationTime         = 0.0f; // current animation time
        RootMotion m_RootMotion;
        RootPose   m_RootPose;

    private:
        std::vector<glm::vec3> m_LocalTranslations;
        std::vector<glm::quat> m_LocalRotations;
        std::vector<glm::vec3> m_LocalScales;
    };
} // namespace Radiant::Animation