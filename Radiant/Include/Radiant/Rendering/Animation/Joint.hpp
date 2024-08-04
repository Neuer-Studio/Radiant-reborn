#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <string>
#include <string_view>
#include <vector>

namespace Radiant::Animation
{
    struct JointInformation
    {
        std::string             JointName;
        std::optional<uint32_t> ParentJointIndex;

        // rest pose of skeleton. All in bone-local space (i.e. translation/rotation/scale relative to parent)
        glm::vec3 JointTranslation;
        glm::quat JointRotation;
        glm::vec3 JointScale;
    };

    class Joints
    {

   public:
        explicit Joints( uint32_t size );
        Joints() = default;

        [[nodiscard]] uint32_t                AddJoint(const JointInformation& jointInformation );
        [[nodiscard]] std::optional<uint32_t> GetJointIndex( const std::string_view name ) const;

        [[nodiscard]] std::optional<uint32_t> GetParentJointIndex( const uint32_t jointIndex ) const
        {
            RADIANT_VERIFY( jointIndex < m_JointsInformation.size(),
                            "Joint index out of range in Joints::GetParentIndex()!" );
            return m_JointsInformation[jointIndex].ParentJointIndex;
        }

        [[nodiscard]] uint32_t JointCount() const
        {
            return static_cast<uint32_t>(m_JointsInformation.size() );
        }

        [[nodiscard]] const std::string& GetJointName( const uint32_t jointIndex ) const
        {
            RADIANT_VERIFY( jointIndex < m_JointsInformation.size(),
                            "bone index out of range in Skeleton::GetBoneName()!" );
            return m_JointsInformation[jointIndex].JointName;
        }

        [[nodiscard]] const auto& GetJointInfo(uint32_t index) const
        {
            return m_JointsInformation[index];
        }

        bool operator==( const Joints& other ) const;

        bool operator!=( const Joints& other ) const
        {
            return !( *this == other );
        }

   private:
        std::vector<JointInformation> m_JointsInformation;
    };
}; // namespace Radiant::Animation
