#include <Radiant/Rendering/Animation/AnimationController.hpp>
#include <Radiant/Core/Math/Interpolate.hpp>

#include <glm/gtx/quaternion.hpp>

#include <Radiant/Rendering/Mesh.hpp>

namespace Radiant::Animation
{
    AnimationController::AnimationController( const Animation& animation, const Joints& joints,
                                              std::unordered_map<std::string, BoneInfo> map )
         : m_Animation( animation ), m_Joints( joints ), mmap( map )
    {
        m_FinalBoneMatrices.resize( 100, glm::mat4( 1.0f ) );
        m_ParentFinalBoneMatrices.resize( 100, glm::mat4( 1.0f ) );
    }

    void AnimationController::SetAnimation( const Animation& animation )
    {
        m_Animation   = animation;
        m_CurrentTime = 0.0f;
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
    //

    void AnimationController::CalculateBoneTransform( uint32_t indexIterator )
    {
        glm::mat4 nodeTransform = glm::mat4( 1.0 );

        if ( indexIterator >= m_Joints.JointCount() - 1 )
            return;

        if ( mmap.find( m_Joints.GetJointName( indexIterator ) ) != mmap.end() )
        {
            const auto& nodeTransformOpt = UpdateTransforms( m_CurrentTime, indexIterator );
            if ( nodeTransformOpt )
            {
                nodeTransform = nodeTransformOpt.value();
            }
            // else
            //{
            //     nodeTransform = m_Joints.GetFinalTransform(indexIterator);
            // }

            glm::mat4 globalTransformation = GetParrentTransform( indexIterator ) * nodeTransform;

            int       index                  = mmap[m_Joints.GetJointName( indexIterator )].ID;
            glm::mat4 offset                 = mmap[m_Joints.GetJointName( indexIterator )].BoneOffset;
            m_FinalBoneMatrices[index]       = m_sdf * globalTransformation * offset;
            m_ParentFinalBoneMatrices[index] = globalTransformation;
        }
        CalculateBoneTransform( ++indexIterator );
    }

    void AnimationController::UpdateAnimation( Timestep ts )
    {
        // if ( m_Animation )
        {

            m_CurrentTime += 30.0f * ts;
            m_CurrentTime = fmod( m_CurrentTime, m_Animation.GetDuration() );
            CalculateBoneTransform( 0 );
        }
    }

    glm::mat4 AnimationController::GetParrentTransform( uint32_t jointID )
    {
        const auto& parrent = m_Joints.GetParentJointIndex( jointID );
        if ( !parrent )
        {
            return glm::mat4( 1.0 );
        }
        return m_ParentFinalBoneMatrices[*parrent];
    }

    const std::optional<glm::mat4> AnimationController::GetBoneUpdateTransform(uint32_t boneID)
    {
        return UpdateTransforms(m_CurrentTime, boneID);
    }

} // namespace Radiant::Animation