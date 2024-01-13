#pragma once

#include <Radiant/Rendering/VertexBuffer.hpp>
#include <Radiant/Core/Memory/Buffer.hpp>

namespace Radiant
{
	class OpenGLVertexBuffer : public VertexBuffer 
	{
	public:
		OpenGLVertexBuffer(const std::byte* data, uint32_t size, OpenGLBufferUsage usage = OpenGLBufferUsage::Static);
		OpenGLVertexBuffer(uint32_t size, OpenGLBufferUsage usage = OpenGLBufferUsage::Dynamic);

		virtual void SetData() override;
		virtual void Use() const override;

		virtual unsigned int GetSize() const override { return m_Buffer.Size; }
		virtual RenderingID GetRenderingID() const override { return m_RenderingID; }
	private:
		RenderingID m_RenderingID = -1;
		OpenGLBufferUsage m_Usage;
		Memory::Buffer m_Buffer;
	};
}