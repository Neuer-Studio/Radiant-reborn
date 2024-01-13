#pragma once

#include <Radiant/Rendering/RenderingTypes.hpp>

namespace Radiant
{
	class IndexBuffer : public Memory::RefCounted
	{
	public:
		virtual ~IndexBuffer() = default;
		virtual void SetData() = 0;
		virtual void Use() const = 0;

		virtual unsigned int GetSize() const = 0;
		virtual unsigned int GetCount() const = 0;
		virtual RenderingID GetRenderingID() const = 0;

		static Memory::Shared<IndexBuffer> Create(const std::byte* data, uint32_t size, OpenGLBufferUsage usage = OpenGLBufferUsage::Static);
		static Memory::Shared<IndexBuffer> Create(uint32_t size, OpenGLBufferUsage usage = OpenGLBufferUsage::Dynamic);

	};
}