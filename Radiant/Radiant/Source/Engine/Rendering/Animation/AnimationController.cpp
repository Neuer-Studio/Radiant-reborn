#include <Radiant/Rendering/Animation/AnimationController.hpp>
#include <Common/Core/Math/Interpolate.hpp>

#include <glm/gtx/quaternion.hpp>

#include <Radiant/Rendering/Mesh.hpp>

namespace Radiant::Animation
{
    AnimationController::AnimationController( const Animation& animation, const Skeleton& skeleton )
         : m_Animation( animation ), m_Skeleton( skeleton )
    {
        m_TranslationCache.Resize( skeleton.BoneCount() );
        m_RotationCache.Resize( skeleton.BoneCount() );
        m_ScaleCache.Resize( skeleton.BoneCount() );
        m_LocalTranslations = skeleton.ListTranslations();
        m_LocalRotations    = skeleton.ListRotations();
        m_LocalScales       = skeleton.ListScales();
    }

    void AnimationController::SetAnimation( const Animation& animation )
    {
        m_Animation     = animation;
        m_AnimationTime = 0.0f;
    }

    void AnimationController::OnUpdate( Common::Timestep ts )
    {
        if ( m_IsAnimationPlaying )
        {
            SampleAnimation();
            m_AnimationTime += ts * GetPlaybackSpeed() / m_Animation.GetDuration();
            m_AnimationTime = m_AnimationTime - floorf( m_AnimationTime );
            m_AnimationTime = fmod( m_AnimationTime, m_Animation.GetDuration() );
        }

        m_PreviousAnimationTime = m_AnimationTime;
    }

    void AnimationController::SampleAnimation()
    {
        m_TranslationCache.Step( m_AnimationTime, m_Animation.GetTranslationKeys() );
        m_RotationCache.Step( m_AnimationTime, m_Animation.GetRotationKeys() );
        m_ScaleCache.Step( m_AnimationTime, m_Animation.GetScaleKeys() );

        m_TranslationCache.Interpolate( m_AnimationTime, m_LocalTranslations,
                                        []( const glm::vec3& a, const glm::vec3& b, const float t )
                                        { return glm::mix( a, b, t ); } );
        m_RotationCache.Interpolate( m_AnimationTime, m_LocalRotations,
                                     []( const glm::quat& a, const glm::quat& b, const float t )
                                     { return glm::slerp( a, b, t ); } );
        m_ScaleCache.Interpolate( m_AnimationTime, m_LocalScales,
                                  []( const glm::vec3& a, const glm::vec3& b, const float t )
                                  { return glm::mix( a, b, t ); } );

        RootPose rootPoseNew = { m_LocalTranslations[0], m_LocalRotations[0] };

        m_RootPose = rootPoseNew;
    }

} // namespace Radiant::Animation