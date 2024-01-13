#include <Radiant/Rendering/IndexBuffer.hpp>
#include <Radiant/Rendering/Rendering.hpp>
#include <Radiant/Rendering/Platform/OpenGL/OpenGLIndexBuffer.hpp>

namespace Radiant {
	Memory::Shared<IndexBuffer> IndexBuffer::Create(const std::byte* data, uint32_t size, OpenGLBufferUsage usage)
	{
		switch (RenderingAPI::GetAPI())
		{
		case RenderingAPIType::None:    return nullptr;
		case RenderingAPIType::OpenGL:  return Memory::Shared<OpenGLIndexBuffer>::Create(data, size, usage);
		}
		RADIANT_VERIFY(false, "Unknown RenderingAPI");
		return nullptr;
	}

	Memory::Shared<IndexBuffer> IndexBuffer::Create(uint32_t size, OpenGLBufferUsage usage)
	{
		switch (RenderingAPI::GetAPI())
		{
		case RenderingAPIType::None:    return nullptr;
		case RenderingAPIType::OpenGL:  return Memory::Shared<OpenGLIndexBuffer>::Create(size, usage);
		}
		RADIANT_VERIFY(false, "Unknown RenderingAPI");
		return nullptr;
	}
}