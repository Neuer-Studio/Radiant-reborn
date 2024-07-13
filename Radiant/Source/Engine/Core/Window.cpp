#include <Core/Window.hpp>

#if defined(RADIANT_PLATFORM_WINDOWS)
#include <Platform/Windows/WindowsWindow.hpp>
#elif defined(RADIANT_PLATFORM_MACOS)
#include <Platform/macOS/MacOSWindow.hpp>
#endif

namespace Radiant
{
	Memory::Shared<Window> Window::Create(const WindowSpecification & specification)
	{
#if defined(RADIANT_PLATFORM_WINDOWS)
		return Memory::Shared<WindowsWindow>::Create(specification);
#elif defined(RADIANT_PLATFORM_MACOS)
        return Memory::Shared<MacOSWindow>::Create(specification);
#else
	#error "Current Platform doesn't supports"
#endif

	}
}
