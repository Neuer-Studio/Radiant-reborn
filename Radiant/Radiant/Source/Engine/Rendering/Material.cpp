#include <Radiant/Rendering/Material.hpp>
#include <Radiant/Rendering/Rendering.hpp>
#include <Radiant/Rendering/Platform/OpenGL/OpenGLMaterial.hpp>

namespace Radiant
{
	Common::Memory::Shared<Material> Material::Create(const Common::Memory::Shared<Shader>& shader)
	{
		switch (RendererAPI::GetAPI())
		{
			case RenderingAPIType::None:    return nullptr;
			case RenderingAPIType::OpenGL:  return Common::Memory::Shared<OpenGLMaterial >::Create(shader);
		}
		RADIANT_VERIFY(false, "Unknown Rendering API");
		return nullptr;
	}

}