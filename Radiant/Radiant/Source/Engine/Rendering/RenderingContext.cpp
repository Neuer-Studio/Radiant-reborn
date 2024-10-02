#include <Radiant/Rendering/RenderingContext.hpp>
#include <Radiant/Rendering/RendererAPI.hpp>
#include <Radiant/Rendering/Platform/Vulkan/VulkanRenderingContext.hpp>
#include <Radiant/Rendering/Platform/OpenGL/OpenGLRenderingContext.hpp>

namespace Radiant
{
    Common::Memory::Shared<RenderingContext> RenderingContext::Create( GLFWwindow* window )
    {
        switch ( RendererAPI::GetAPI() )
        {
            case RenderingAPIType::Vulkan:
            {
                return Common::Memory::Shared<VulkanRenderingContext>::Create( window );
            }

            case RenderingAPIType::OpenGL:
            {
                return Common::Memory::Shared<OpenGLRenderingContext>::Create( window );
            }
        }

        RADIANT_VERIFY( false );
        return nullptr;
    }

} // namespace Radiant