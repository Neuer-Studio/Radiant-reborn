#include <Radiant/Rendering/Animation/Joint.hpp>
#include <Radiant/Core/Math/Matrix.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Radiant::Animation
{

    Joints::Joints( uint32_t size )
    {
        m_JointsInformation.reserve( size );
    }

    [[nodiscard]] uint32_t Joints::AddJoint( const JointInformation& jointInformation )
    {
        uint32_t index = m_JointsInformation.size();
        m_JointsInformation.push_back(jointInformation);
        return index;
    }

    [[nodiscard]] std::optional<uint32_t> Joints::GetJointIndex( const std::string_view name ) const
    {
        for ( size_t i = 0; i < m_JointsInformation.size(); ++i )
        {
            if (m_JointsInformation[i].JointName == name )
            {
                return static_cast<uint32_t>( i );
            }
        }
        return std::nullopt;
    }

    bool Joints::operator==( const Joints& other ) const
    {
        bool areSame = false;
        if ( JointCount() == other.JointCount() )
        {
            areSame = true;
            for ( uint32_t i = 0; i < JointCount(); ++i )
            {
                if ( GetJointName( i ) != other.GetJointName( i ) )
                {
                    areSame = false;
                    break;
                }
            }
        }
        return areSame;
    }

} // namespace Radiant::Animation