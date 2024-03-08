#pragma once

#include <Radiant/Rendering/IndexBuffer.hpp>
#include <Radiant/Core/Memory/Buffer.hpp>

namespace Radiant
{
	class OpenGLIndexBuffer : public IndexBuffer
	{
	public:
		OpenGLIndexBuffer(const void*, uint32_t size, OpenGLBufferUsage usage);
		OpenGLIndexBuffer(uint32_t size, OpenGLBufferUsage usage);
		~OpenGLIndexBuffer() { m_Buffer.Release(); }

		virtual void SetData() override;
		virtual void Use(BindUsage use = BindUsage::Bind) const override;

		virtual unsigned int GetSize() const override { return m_Buffer.Size; }
		virtual unsigned int GetCount() const override { return m_Buffer.Size / sizeof(uint32_t); }
		virtual RenderingID GetRenderingID() const override { return m_RenderingID; }
	private:
		RenderingID m_RenderingID = 0;
		OpenGLBufferUsage m_Usage;
		Memory::Buffer m_Buffer;
	};
}