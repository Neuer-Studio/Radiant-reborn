#include <glad/glad.h>

#include <Radiant/Rendering/Platform/OpenGL/OpenGLVertexBuffer.hpp>
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

	OpenGLVertexBuffer::OpenGLVertexBuffer(const std::byte* data, uint32_t size, OpenGLBufferUsage usage)
		: m_Buffer((void*)data, size), m_Usage(usage)
	{
		Memory::Shared<OpenGLVertexBuffer> instance(this);
		Rendering::SubmitCommand([instance]() mutable
			{
				RA_INFO("[OpenGLVertexBuffer] OpenGLVertexBuffer::OpenGLVertexBuffer");

				glGenBuffers(1, &instance->m_RenderingID);
				glBindBuffer(GL_ARRAY_BUFFER, instance->m_RenderingID);
				glBufferData(GL_ARRAY_BUFFER, instance->m_Buffer.Size, instance->m_Buffer.Data, OpenGLUsage(instance->m_Usage));

				glEnableVertexAttribArray(0);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
			});
	}

	OpenGLVertexBuffer::OpenGLVertexBuffer(uint32_t size, OpenGLBufferUsage usage)
		: m_Usage(usage)
	{
		m_Buffer.Allocate(size);
	}

	void OpenGLVertexBuffer::SetData()
	{
		RADIANT_VERIFY(false);
	}

	void OpenGLVertexBuffer::Use() const
	{
		auto id = m_RenderingID;
		Rendering::SubmitCommand([id]()
			{
				glBindBuffer(GL_ARRAY_BUFFER, id);
			});
	}

}