#include <Radiant/Rendering/Animation/Joint.hpp>
#include <Radiant/Core/Math/Matrix.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Radiant::Animation
{

    Joints::Joints( uint32_t size )
    {
        m_JointNames.reserve( size );
        m_ParentJointIndices.reserve( size );
    }

    [[nodiscard]] uint32_t Joints::AddJoint( std::string name, std::optional<uint32_t> parentIndex,
                                             const glm::mat4& transform )
    {
        uint32_t index = static_cast<uint32_t>( m_JointNames.size() );
        m_JointNames.emplace_back( name );
        m_ParentJointIndices.emplace_back( parentIndex );
        m_JointTranslations.emplace_back();
        m_JointRotations.emplace_back();
        m_JointScales.emplace_back();
        m_FinalJointTrasform.emplace_back(transform);

        glm::vec3 skew;
        glm::vec4 perspective;

        glm::decompose( transform, m_JointScales.back(), m_JointRotations.back(), m_JointTranslations.back(), skew,
                        perspective );

        return index;
    }

    [[nodiscard]] std::optional<uint32_t> Joints::GetJointIndex( const std::string_view name ) const
    {
        for ( size_t i = 0; i < m_JointNames.size(); ++i )
        {
            if ( m_JointNames[i] == name )
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