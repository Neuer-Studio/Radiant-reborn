#include <Core/Window.hpp>

#include <Platform/Windows/WindowsWindow.hpp>

namespace Radiant
{
	Ref<Window> Window::Create(const WindowSpecification& specification)
	{
#if defined(RADIANT_PLATFORM_WINDOWS)
		return Ref<Window>(new WindowsWindow(specification));
#else 
	#error "Current Platform doesn't supports"
#endif

	}
}