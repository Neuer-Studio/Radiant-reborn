#include <Radiant/Rendering/Animation/Animation.hpp>

namespace Radiant::Animation
{
    Animation::Animation( const std::string_view name, const float duration )
         : m_Name( name ), m_Duration( duration )
    {
    }

    void Animation::SetKeyFrames( std::vector<TranslationKey> translations, std::vector<RotationKey> rotations,
                                  std::vector<ScaleKey> scales )
    {
        m_TranslationKeys = std::move( translations );
        m_RotationKeys    = std::move( rotations );
        m_ScaleKeys       = std::move( scales );
    }


} // namespace Radiant::Animation