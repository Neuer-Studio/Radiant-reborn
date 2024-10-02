#include <Radiant/Rendering/Texture.hpp>
#include <Radiant/Rendering/Rendering.hpp>
#include <Radiant/Rendering/Platform/OpenGL/OpenGLTexture.hpp>

namespace Radiant
{
    Common::Memory::Shared<Texture2D> Texture2D::Create( const std::filesystem::path& path, bool srgb )
    {
        switch ( RendererAPI::GetAPI() )
        {
            case RenderingAPIType::None:
                return nullptr;
            case RenderingAPIType::OpenGL:
                return Common::Memory::Shared<OpenGLTexture2D>::Create( path, srgb );
        }
        RADIANT_VERIFY( false, "Unknown Rendering API" );
        return nullptr;
    }

    Common::Memory::Shared<Texture2D> Texture2D::Create( const Texture2DCreateInformation& info )
    {
        switch ( RendererAPI::GetAPI() )
        {
            case RenderingAPIType::None:
                return nullptr;
            case RenderingAPIType::OpenGL:
                return Common::Memory::Shared<OpenGLTexture2D>::Create( info );
        }
        RADIANT_VERIFY( false, "Unknown Rendering API" );
        return nullptr;
    }

} // namespace Radiant