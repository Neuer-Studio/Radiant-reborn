#include <glad/glad.h>

#include <Radiant/Rendering/Platform/OpenGL/OpenGLIndexBuffer.hpp>
#include <Radiant/Rendering/Rendering.hpp>

namespace Radiant
{
	static auto OpenGLUsage(OpenGLBufferUsage usage)
	{
		switch (usage)
		{
		case OpenGLBufferUsage::Static:    return GL_STATIC_DRAW;
		case OpenGLBufferUsage::Dynamic:   return GL_DYNAMIC_DRAW;
		}
		RADIANT_VERIFY(false, "Unknown vertex buffer usage");
		return 0;
	}

	OpenGLIndexBuffer::OpenGLIndexBuffer(const std::byte* data, uint32_t size, OpenGLBufferUsage usage)
		: m_Buffer((void*)data, size), m_Usage(usage)
	{
		Memory::Shared<OpenGLIndexBuffer> instance(this);
		Rendering::SubmitCommand([instance]() mutable
			{
				glGenBuffers(1, &instance->m_RenderingID);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, instance->m_RenderingID);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, instance->m_Buffer.Size, instance->m_Buffer.Data, OpenGLUsage(instance->m_Usage));
			});
	}

	OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t size, OpenGLBufferUsage usage)
		: m_Usage(usage)
	{
		m_Buffer.Allocate(size);
	}

	void OpenGLIndexBuffer::SetData()
	{
		RADIANT_VERIFY(false);
	}

	void OpenGLIndexBuffer::Use(BindUsage use) const
	{
		auto id = m_RenderingID;
		Rendering::SubmitCommand([id, use]()
			{
				if (use == BindUsage::Clear)
				{
					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
					return;
				}
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
			});
	}
}