#include <Radiant/Rendering/UniformBuffer.hpp>

#include <Radiant/Rendering/Platform/OpenGL/OpenGLUniformBuffer.hpp>
#include <Radiant/Rendering/Rendering.hpp>
#include <Radiant/Rendering/Platform/OpenGL/OpenGLImage.hpp>

namespace Radiant
{

    Radiant::Memory::Shared<Radiant::UniformBuffer> UniformBuffer::Create( uint32_t size, uint32_t binding )
    {
        switch ( RendererAPI::GetAPI() )
        {
            case RenderingAPIType::None:
                return nullptr;
            case RenderingAPIType::OpenGL:
                return Memory::Shared<OpenGLUniformBuffer>::Create( size, binding );
        }
        RADIANT_VERIFY( false, "Unknown Rendering API" );
        return nullptr;
    }

} // namespace Radiant