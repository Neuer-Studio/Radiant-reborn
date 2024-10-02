#pragma once

namespace Radiant::Assets
{
    struct AssetMetadata
    {
        std::optional<Common::AssetHandle> Handle;

        std::filesystem::path FilePath;

        bool IsValid() const
        {
            return Handle.has_value();
        }
    };
} // namespace Radiant::Assets