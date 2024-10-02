#include <Radiant/Assets/AssetRegestry.hpp>

namespace Radiant::Assets
{

    std::optional<AssetMetadata> AssetRegestry::GetMetadata( const Common::AssetHandle& handle ) const
    {
        if ( m_AssetsInfo.find( handle ) == m_AssetsInfo.end() ) [[unlikely]]
        {
            return std::nullopt;
        }
        else [[likely]]
        {
            return m_AssetsInfo.at( handle );
        }
    }

} // namespace Radiant::Assets