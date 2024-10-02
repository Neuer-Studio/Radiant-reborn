#pragma once

#include "AssetMetadata.hpp"
#include "AssetRegestry.hpp"

namespace Radiant::Assets
{
    class AssetManager final : public Common::Memory::RefCounted
    {
    public:
        static void                                              Init();
        static void                                              Shutdown();
        [[nodiscard]] static const std::optional<AssetMetadata>& GetMetadata( const Common::AssetHandle& handle );

        ~AssetManager() = default;

    private:
        inline static AssetRegestry* s_AssetRegestry;
    };
} // namespace Radiant::Assets