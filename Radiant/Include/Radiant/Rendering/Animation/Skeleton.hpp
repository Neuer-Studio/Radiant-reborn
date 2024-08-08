#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <string>
#include <string_view>
#include <vector>

namespace Radiant::Animation
{
    struct BoneInformation
    {
        std::string             BoneName;
        std::optional<uint32_t> ParentBoneIndex;

        // rest pose of skeleton. All in bone-local space (i.e. translation/rotation/scale relative to parent)
        glm::vec3 BoneTranslation;
        glm::quat BoneRotation;
        glm::vec3 BoneScale;
    };

    class Skeleton
    {

   public:
        explicit Skeleton( uint32_t size );
        Skeleton() = default;

        [[nodiscard]] uint32_t                AddBone(const BoneInformation& BoneInformation );
        [[nodiscard]] std::optional<uint32_t> GetBoneIndex( const std::string_view name ) const;

        [[nodiscard]] std::optional<uint32_t> GetParentBoneIndex( const uint32_t boneIndex) const
        {
            RADIANT_VERIFY( boneIndex < m_BonesInformation.size(),
                            "Bone index out of range in Bones::GetParentIndex()!" );
            return m_BonesInformation[boneIndex].ParentBoneIndex;
        }

        [[nodiscard]] std::vector<glm::vec3> ListTranslations() const
        {
            std::vector< glm::vec3> temp;
            for (const auto& bone : m_BonesInformation)
            {
                temp.push_back(bone.BoneTranslation);
            }
            return temp;
        }

        [[nodiscard]] std::vector<glm::vec3> ListScales() const
        {
            std::vector<glm::vec3> temp;
            for ( const auto& bone : m_BonesInformation )
            {
                temp.push_back( bone.BoneScale );
            }
            return temp;
        }

        [[nodiscard]] std::vector<glm::quat> ListRotations() const
        {
            std::vector<glm::quat> temp;
            for ( const auto& bone : m_BonesInformation )
            {
                temp.push_back( bone.BoneRotation );
            }
            return temp;
        }

        [[nodiscard]] uint32_t BoneCount() const
        {
            return static_cast<uint32_t>(m_BonesInformation.size() );
        }

        [[nodiscard]] const std::string& GetBoneName( const uint32_t boneIndex) const
        {
            RADIANT_VERIFY(boneIndex < m_BonesInformation.size(),
                            "bone index out of range in Skeleton::GetBoneName()!" );
            return m_BonesInformation[boneIndex].BoneName;
        }

        [[nodiscard]] const auto& GetBoneInfo(uint32_t index) const
        {
            return m_BonesInformation[index];
        }

        bool operator==( const Skeleton& other ) const;

        bool operator!=( const Skeleton& other ) const
        {
            return !( *this == other );
        }

   private:
        std::vector<BoneInformation> m_BonesInformation;
    };
}; // namespace Radiant::Animation
