#include <Common/Core/Window.hpp>

#include <Platform/Windows/WindowsWindow.hpp>

namespace Common
{
    Memory::Shared<Window>
    Window::Create( const WindowSpecification& specification )
    {
#if defined( RADIANT_PLATFORM_WINDOWS )
        return Memory::Shared<Radiant::WindowsWindow>::Create( specification );
#else
#error "Current Platform doesn't supports"
#endif
    }
} // namespace Radiant