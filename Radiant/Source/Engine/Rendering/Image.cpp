#include <Radiant/Rendering/Image.hpp>
#include <Radiant/Rendering/Rendering.hpp>
#include <Radiant/Rendering/Platform/OpenGL/OpenGLImage.hpp>

namespace Radiant
{

	Radiant::Memory::Shared<Radiant::Image2D> Image2D::Create(const ImageSpecification& spec)
	{
		switch (RenderingAPI::GetAPI())
		{
			case RenderingAPIType::None:    return nullptr;
			case RenderingAPIType::OpenGL:  return Memory::Shared<OpenGLImage2D>::Create(spec);
		}
		RADIANT_VERIFY(false, "Unknown Rendering API");
		return nullptr;
	}

}