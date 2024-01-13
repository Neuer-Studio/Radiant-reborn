#include <Radiant/Rendering/VertexBuffer.hpp>
#include <Radiant/Rendering/Rendering.hpp>
#include <Radiant/Rendering/Platform/OpenGL/OpenGLVertexBuffer.hpp>

namespace Radiant {

	Memory::Shared<VertexBuffer> VertexBuffer::Create(std::byte* data, uint32_t size, OpenGLBufferUsage usage)
	{
		switch (RenderingAPI::GetAPI())
		{
			case RenderingAPIType::OpenGL:
			{
				return Memory::Shared<OpenGLVertexBuffer>::Create(data, size, usage);
			}
		}

		RADIANT_VERIFY(false);
		return nullptr;
	}

	Memory::Shared<VertexBuffer> VertexBuffer::Create(uint32_t size, OpenGLBufferUsage usage)
	{
		switch (RenderingAPI::GetAPI())
		{
		case RenderingAPIType::OpenGL:
		{
			return Memory::Shared<OpenGLVertexBuffer>::Create(size, usage);
		}
		}

		RADIANT_VERIFY(false);
		return nullptr;
	}

	uint32_t VertexBufferElement::GetComponentCount() const
	{
		switch (Type)
		{
		case ShaderDataType::Float:   return 1;
		case ShaderDataType::Float2:  return 2;
		case ShaderDataType::Float3:  return 3;
		case ShaderDataType::Float4:  return 4;
		case ShaderDataType::Int:     return 1;
		case ShaderDataType::Int2:    return 2;
		case ShaderDataType::Int3:    return 3;
		case ShaderDataType::Int4:    return 4;
		case ShaderDataType::Bool:    return 1;
		}

		RADIANT_VERIFY(false, "Unknown ShaderDataType!");
		return 0;
	}

}