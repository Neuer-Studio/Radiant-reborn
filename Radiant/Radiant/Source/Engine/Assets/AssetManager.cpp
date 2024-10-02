#include <Radiant/Assets/AssetManager.h>

namespace Radiant::Assets
{

    void AssetManager::Init()
    {
        s_AssetRegestry = new AssetRegestry();
    }

    void AssetManager::Shutdown()
    {
        delete s_AssetRegestry;
    }

    [[nodiscard]] const std::optional<Radiant::Assets::AssetMetadata>&
    AssetManager::GetMetadata( const Common::AssetHandle& handle )
    {
        return s_AssetRegestry->GetMetadata( handle );
    }

} // namespace Radiant::Assets