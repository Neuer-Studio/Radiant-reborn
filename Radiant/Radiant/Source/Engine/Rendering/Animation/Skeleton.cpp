#include <Radiant/Rendering/Animation/Skeleton.hpp>
#include <Radiant/Core/Math/Matrix.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Radiant::Animation
{

    Skeleton::Skeleton( uint32_t size )
    {
        m_BonesInformation.reserve( size );
    }

    [[nodiscard]] uint32_t Skeleton::AddBone( const BoneInformation& jointInformation )
    {
        uint32_t index = m_BonesInformation.size();
        m_BonesInformation.push_back(jointInformation);
        return index;
    }

    [[nodiscard]] std::optional<uint32_t> Skeleton::GetBoneIndex( const std::string_view name ) const
    {
        for ( size_t i = 0; i < m_BonesInformation.size(); ++i )
        {
            if (m_BonesInformation[i].BoneName == name )
            {
                return static_cast<uint32_t>( i );
            }
        }
        return std::nullopt;
    }

    bool Skeleton::operator==( const Skeleton& other ) const
    {
        bool areSame = false;
        if ( BoneCount() == other.BoneCount() )
        {
            areSame = true;
            for ( uint32_t i = 0; i < BoneCount(); ++i )
            {
                if ( GetBoneName( i ) != other.GetBoneName( i ) )
                {
                    areSame = false;
                    break;
                }
            }
        }
        return areSame;
    }

} // namespace Radiant::Animation