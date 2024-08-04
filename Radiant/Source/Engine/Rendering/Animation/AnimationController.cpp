#include <Radiant/Rendering/Animation/AnimationController.hpp>
#include <Radiant/Core/Math/Interpolate.hpp>

#include <glm/gtx/quaternion.hpp>

#include <Radiant/Rendering/Mesh.hpp>

namespace Radiant::Animation
{
    AnimationController::AnimationController( const Animation& animation, const Joints& joints)
         : m_Animation( animation ), m_Joints( joints )
    {
    }

    void AnimationController::SetAnimation( const Animation& animation )
    {
        m_Animation   = animation;
        m_AnimationTime = 0.0f;
    }

    std::optional<glm::mat4> AnimationController::UpdateTransforms( float animationTime, uint32_t jointID )
    {
        const auto& translation = InterpolatePosition( animationTime, jointID );
        const auto& rotation    = InterpolateRotation( animationTime, jointID );
        const auto& scale       = InterpolateScaling( animationTime, jointID );

        if ( !translation || !rotation || !scale )
        {
            return std::nullopt;
        }
        return ( *translation ) * ( *rotation ) * ( *scale ); // NOTE: sholud use .value();
    }

    //========================================================================//

    static float GetScaleFactor( float lastTimeStamp, float nextTimeStamp, float animationTime )
    {
        float scaleFactor  = 0.0f;
        float midWayLength = animationTime - lastTimeStamp;
        float framesDiff   = nextTimeStamp - lastTimeStamp;
        scaleFactor        = midWayLength / framesDiff;
        return scaleFactor;
    }

    std::optional<glm::mat4> AnimationController::InterpolatePosition( float animationTime, uint32_t jointID )
    {
        if ( m_Animation.GetTranslationKeys().size() == 1 )
        {
            return glm::translate( glm::mat4( 1.0f ), m_Animation.GetTranslationKeys()[0].Value );
        }

        const auto& pair = FindIndexAndGetPair<TranslationKey, TranslationKey>( m_Animation.GetTranslationKeys(),
                                                                                animationTime, jointID );
        if ( !pair )
        {
            return std::nullopt;
        }

        float scaleFactor = GetScaleFactor( pair->first.FrameTime, pair->second.FrameTime, animationTime );
        return Math::InterpolateTranslation( pair->first.Value, pair->second.Value, scaleFactor );
    }

    std::optional<glm::mat4> AnimationController::InterpolateRotation( float animationTime, uint32_t jointID )
    {
        if ( m_Animation.GetRotationKeys().size() == 1 )
        {
            auto rotation = glm::normalize( m_Animation.GetRotationKeys()[0].Value );
            return glm::toMat4( rotation );
        }

        const auto& pair = FindIndexAndGetPair<RotationKey, RotationKey>( m_Animation.GetRotationKeys(),
                                                                          animationTime, jointID );
        if ( !pair )
        {
            return std::nullopt;
        }

        float scaleFactor = GetScaleFactor( pair->first.FrameTime, pair->second.FrameTime, animationTime );
        return Math::InterpolateRotation( pair->first.Value, pair->second.Value, scaleFactor );
    }

    std::optional<glm::mat4> AnimationController::InterpolateScaling( float animationTime, uint32_t jointID )
    {
        if ( m_Animation.GetScaleKeys().size() == 1 )
        {
            return glm::scale( glm::mat4( 1.0f ), m_Animation.GetScaleKeys()[0].Value );
        }

        const auto& pair =
             FindIndexAndGetPair<ScaleKey, ScaleKey>( m_Animation.GetScaleKeys(), animationTime, jointID );
        if ( !pair )
        {
            return std::nullopt;
        }

        float scaleFactor = GetScaleFactor( pair->first.FrameTime, pair->second.FrameTime, animationTime );
        return Math::InterpolateScale( pair->first.Value, pair->second.Value, scaleFactor );
    }

    void AnimationController::OnUpdate( Timestep ts )
    {
        if (m_IsAnimationPlaying)
        {

            m_AnimationTime += ts * GetPlaybackSpeed() / m_Animation.GetDuration();
            m_AnimationTime = m_AnimationTime - floorf(m_AnimationTime);
            m_AnimationTime = fmod(m_AnimationTime, m_Animation.GetDuration() );
        }
    }

    const std::optional<glm::mat4> AnimationController::GetBoneUpdateTransform(uint32_t boneID)
    {
        return UpdateTransforms(m_AnimationTime, boneID);
    }

} // namespace Radiant::Animation