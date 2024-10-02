#pragma once

#include "AssetTypes.hpp"

namespace Radiant::Assets
{
    class Asset : public Common::Memory::RefCounted
    {
    public:
        Asset() = default;

        virtual ~Asset() = default;

        virtual AssetType GetAssetType() const
        {
            return AssetType::None;
        }

        auto GetAssetHandle() -> Common::AssetHandle const
        {
            return m_Handle;
        }

    private:
        Common::AssetHandle m_Handle;
    };
} // namespace Radiant::Assets

#define REGESTRY_ASSET_TYPE( type )                                                                               \
    static Radiant::Assets::AssetType GetStaticType()                                                             \
    {                                                                                                             \
        return Radiant::Assets::AssetType::type;                                                                  \
    }                                                                                                             \
    virtual Radiant::Assets::AssetType GetAssetType() const override                                              \
    {                                                                                                             \
        return GetStaticType();                                                                                   \
    }

#define REGESTRY_AS_ASSET                                                                                         \
 : public Radiant::Assets::Asset