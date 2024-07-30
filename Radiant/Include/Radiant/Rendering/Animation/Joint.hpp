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

        [[nodiscard]] uint32_t                AddJoint( std::string name, std::optional<uint32_t> parentIndex,
                                                        const glm::mat4& transform );
        [[nodiscard]] std::optional<uint32_t> GetJointIndex( const std::string_view name ) const;

        [[nodiscard]] std::optional<uint32_t> GetParentJointIndex( const uint32_t jointIndex ) const
        {
            RADIANT_VERIFY( jointIndex < m_ParentJointIndices.size(),
                            "Joint index out of range in Joints::GetParentIndex()!" );
            return m_ParentJointIndices[jointIndex];
        }

        [[nodiscard]] uint32_t JointCount() const
        {
            return static_cast<uint32_t>( m_JointNames.size() );
        }

        [[nodiscard]] const std::string& GetJointName( const uint32_t jointIndex ) const
        {
            RADIANT_VERIFY( jointIndex < m_JointNames.size(),
                            "bone index out of range in Skeleton::GetBoneName()!" );
            return m_JointNames[jointIndex];
        }

        [[nodiscard]] const auto& ListJointNames() const
        {
            return m_JointNames;
        }
        [[nodiscard]] const std::vector<glm::vec3> GetJointTranslations() const
        {
            return m_JointTranslations;
        }
        [[nodiscard]] const std::vector<glm::quat> GetJointRotations() const
        {
            return m_JointRotations;
        }
        [[nodiscard]] const std::vector<glm::vec3> GetJointScales() const
        {
            return m_JointScales;
        }
        [[nodiscard]] const glm::mat4 GetFinalTransform(uint32_t index) const
        {
            return m_FinalJointTrasform[index];
        }

        bool operator==( const Joints& other ) const;

        bool operator!=( const Joints& other ) const
        {
            return !( *this == other );
        }

   private:
        std::vector<std::string>             m_JointNames;
        std::vector<std::optional<uint32_t>> m_ParentJointIndices;

        std::vector<glm::mat4> m_FinalJointTrasform;

        // rest pose of skeleton. All in bone-local space (i.e. translation/rotation/scale relative to parent)
        std::vector<glm::vec3> m_JointTranslations;
        std::vector<glm::quat> m_JointRotations;
        std::vector<glm::vec3> m_JointScales;
    };
}; // namespace Radiant::Animation
