#include <Radiant/Rendering/Environment.hpp>
#include <Radiant/Rendering/SceneRendering.hpp>

namespace Radiant
{
    Environment Environment::Create( const std::filesystem::path& filepath )
    {
        return SceneRendering::CreateEnvironmentMap( filepath );
    }
} // namespace Radiant