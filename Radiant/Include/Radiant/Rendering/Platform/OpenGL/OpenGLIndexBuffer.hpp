#pragma once

#include <Radiant/Rendering/IndexBuffer.hpp>
#include <Radiant/Core/Memory/Buffer.hpp>

namespace Radiant
{
	class OpenGLIndexBuffer : public IndexBuffer
	{
	public:
		OpenGLIndexBuffer(const std::byte* data, uint32_t size, OpenGLBufferUsage usage);
		OpenGLIndexBuffer(uint32_t size, OpenGLBufferUsage usage);

		virtual void SetData() override;
		virtual void Use() const override;

		virtual unsigned int GetSize() const override { return m_Buffer.Size / sizeof(uint32_t); }
		virtual unsigned int GetCount() const override { return m_Buffer.Size; }
		virtual RenderingID GetRenderingID() const override { return m_RenderingID; }
	private:
		RenderingID m_RenderingID = -1;
		OpenGLBufferUsage m_Usage;
		Memory::Buffer m_Buffer;
	};
}