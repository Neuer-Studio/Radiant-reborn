#pragma once

#include <Radiant/Rendering/Animation/Skeleton.hpp>
#include <Radiant/Rendering/Animation/BoneInfo.hpp>

namespace Radiant::Animation
{
    template <typename T>
    struct AnimationKey
    {
        T        Value;
        float    FrameTime; // 0.0f = beginning of animation clip, 1.0f = end of animation clip
        uint32_t Track;

        AnimationKey( const float frameTime, const uint32_t track, const T& value )
             : FrameTime( frameTime ), Track( track ), Value( value )
        {
        }
    };
    using TranslationKey = AnimationKey<glm::vec3>;
    using RotationKey    = AnimationKey<glm::quat>;
    using ScaleKey       = AnimationKey<glm::vec3>;

    class Animation
    {
   public:
        Animation( const std::string_view name, const float duration );

        const std::string& GetName() const
        {
            return m_Name;
        }
        float GetDuration() const
        {
            return m_Duration;
        }

        void SetKeyFrames( std::vector<TranslationKey> translations, std::vector<RotationKey> rotations,
                           std::vector<ScaleKey> scales );

        const auto& GetTranslationKeys() const
        {
            return m_TranslationKeys;
        }
        const auto& GetRotationKeys() const
        {
            return m_RotationKeys;
        }
        const auto& GetScaleKeys() const
        {
            return m_ScaleKeys;
        }
   private:
        std::vector<TranslationKey> m_TranslationKeys;
        std::vector<RotationKey>    m_RotationKeys;
        std::vector<ScaleKey>       m_ScaleKeys;
        std::string                 m_Name;
        float                       m_Duration;
    };

} // namespace Radiant::Animation