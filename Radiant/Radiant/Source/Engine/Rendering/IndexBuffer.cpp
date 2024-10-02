#include <Radiant/Rendering/IndexBuffer.hpp>
#include <Radiant/Rendering/Rendering.hpp>
#include <Radiant/Rendering/Platform/OpenGL/OpenGLIndexBuffer.hpp>

namespace Radiant
{
    Common::Memory::Shared<IndexBuffer> IndexBuffer::Create( const void* data, uint32_t size,
                                                             OpenGLBufferUsage usage )
    {
        switch ( RendererAPI::GetAPI() )
        {
            case RenderingAPIType::None:
                return nullptr;
            case RenderingAPIType::OpenGL:
                return Common::Memory::Shared<OpenGLIndexBuffer>::Create( data, size, usage );
        }
        RADIANT_VERIFY( false, "Unknown RenderingAPI" );
        return nullptr;
    }

    Common::Memory::Shared<IndexBuffer> IndexBuffer::Create( uint32_t size, OpenGLBufferUsage usage )
    {
        switch ( RendererAPI::GetAPI() )
        {
            case RenderingAPIType::None:
                return nullptr;
            case RenderingAPIType::OpenGL:
                return Common::Memory::Shared<OpenGLIndexBuffer>::Create( size, usage );
        }
        RADIANT_VERIFY( false, "Unknown RenderingAPI" );
        return nullptr;
    }
} // namespace Radiant