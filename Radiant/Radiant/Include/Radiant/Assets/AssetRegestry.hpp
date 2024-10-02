#pragma once
#pragma once

#include "AssetMetadata.hpp"

namespace Radiant::Assets
{
    class AssetRegestry final
    {
    public:
        ~AssetRegestry()
        {
            RA_DEBUG( "123123123123123" )
        }
        void RegestryAsset( const AssetMetadata& metadata )
        {
        }
        [[nodiscard]] std::optional<AssetMetadata> GetMetadata( const Common::AssetHandle& handle ) const;

        [[nodiscard]] uint32_t Count() const
        {
            return m_AssetsInfo.size();
        }

    private:
        std::unordered_map<Common::AssetHandle, AssetMetadata> m_AssetsInfo;
    };
} // namespace Radiant::Assets
