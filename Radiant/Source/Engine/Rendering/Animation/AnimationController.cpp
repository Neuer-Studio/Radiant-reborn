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

    void AnimationController::CalculateBoneTransform( uint32_t indexIterator ) //NOTE: Проблема скорее связана с тем, что parentTransform передается просто как предыдущий параметр. 
// Например, мы расчитали JOINT_CORE, после для некоторых дочерних типа JOINT_AA -> JOINT_AA_END, получили матрицу трансформации, которую так же применяем уже к другим костям, а нужно JOINT_CORE
    {
        glm::mat4 nodeTransform = glm::mat4( 1.0 );

        if ( indexIterator >= m_Joints.JointCount() - 1 )
            return;

        if (mmap.find(m_Joints.GetJointName(indexIterator)) != mmap.end())
        {
            const auto& nodeTransformOpt =
                UpdateTransforms(m_CurrentTime, mmap[m_Joints.GetJointName(indexIterator)].ID);
            if (nodeTransformOpt)
            {
                nodeTransform = nodeTransformOpt.value(); // NOTE: вы возвращаем nullopt, если нужный нам
                // сустав не был найден, для того чтобы не искать его
                // в маппере, а оставить его локальную трансформацию
            }

            glm::mat4 globalTransformation = GetParrentTransform(indexIterator) * nodeTransform;

            int       index = mmap[m_Joints.GetJointName(indexIterator)].ID;
            glm::mat4 offset = mmap[m_Joints.GetJointName(indexIterator)].BoneOffset;
            m_FinalBoneMatrices[index] = globalTransformation * offset;
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

    glm::mat4 AnimationController::GetParrentTransform(uint32_t jointID)
    {
        const auto& parrent = m_Joints.GetParentJointIndex(jointID);
        if (!parrent)
        {
            return glm::mat4(1.0);
        }
        return m_FinalBoneMatrices[*parrent];
    }

} // namespace Radiant::Animation