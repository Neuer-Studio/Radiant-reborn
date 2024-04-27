#include <Radiant/Rendering/Material.hpp>
#include <Radiant/Rendering/Rendering.hpp>
#include <Radiant/Rendering/Platform/OpenGL/OpenGLMaterial.hpp>

namespace Radiant
{
	void Material::SetUBOMember(BindingPoint binding, const std::string& name, const glm::vec3& value)
	{
		switch (RendererAPI::GetAPI())
		{
		case RenderingAPIType::OpenGL:
		{
			OpenGLMaterial::SetUBOMember(binding, name, value);
			return;
		}
		default: RADIANT_VERIFY(false, "Unknown Rendering API");
		}
	}

	void Material::SetUBOMember(BindingPoint binding, const std::string& name, const glm::vec2& value)
	{
		switch (RendererAPI::GetAPI())
		{
		case RenderingAPIType::OpenGL:
		{
			OpenGLMaterial::SetUBOMember(binding, name, value);
			return;
		}
		default: RADIANT_VERIFY(false, "Unknown Rendering API");
		}
	}

	void Material::SetUBOMember(BindingPoint binding, const std::string& name, const glm::mat4& value)
	{
		switch (RendererAPI::GetAPI())
		{
		case RenderingAPIType::OpenGL:
		{
			OpenGLMaterial::SetUBOMember(binding, name, value);
			return;
		}
		default: RADIANT_VERIFY(false, "Unknown Rendering API");
		}
	}

	void Material::SetUBOMember(BindingPoint binding, const std::string& name, float value)
	{
		switch (RendererAPI::GetAPI())
		{
		case RenderingAPIType::OpenGL:
		{
			OpenGLMaterial::SetUBOMember(binding, name, value);
			return;
		}
		default: RADIANT_VERIFY(false, "Unknown Rendering API");
		}
	}

	void Material::SetUBOMember(BindingPoint binding, const std::string& name, bool value)
	{
		switch (RendererAPI::GetAPI())
		{
		case RenderingAPIType::OpenGL:
		{
			OpenGLMaterial::SetUBOMember(binding, name, value);
			return;
		}
		default: RADIANT_VERIFY(false, "Unknown Rendering API");
		}
	}

	void Material::SetUBO(BindingPoint binding, const void* data, std::size_t size, std::size_t offset)
	{
		switch (RendererAPI::GetAPI())
		{
		case RenderingAPIType::OpenGL:
		{
			OpenGLMaterial::SetUBO(binding, data, size, offset);
			return;
		}
		default: RADIANT_VERIFY(false, "Unknown Rendering API");
		}
	}

	Memory::Shared<Material> Material::Create(const Memory::Shared<Shader>& shader)
	{
		switch (RendererAPI::GetAPI())
		{
			case RenderingAPIType::None:    return nullptr;
			case RenderingAPIType::OpenGL:  return Memory::Shared<OpenGLMaterial >::Create(shader);
		}
		RADIANT_VERIFY(false, "Unknown Rendering API");
		return nullptr;
	}

}