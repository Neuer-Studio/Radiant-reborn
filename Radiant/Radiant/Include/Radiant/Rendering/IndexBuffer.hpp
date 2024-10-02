#pragma once

#include <Radiant/Rendering/RenderingTypes.hpp>

namespace Radiant
{
    class IndexBuffer : public Common::Memory::RefCounted
    {
    public:
        virtual ~IndexBuffer()                                       = default;
        virtual void SetData()                                       = 0;
        virtual void Use( BindUsage use = BindUsage::Bind ) const    = 0;
        virtual void RT_Use( BindUsage use = BindUsage::Bind ) const = 0;

        virtual unsigned int GetSize() const        = 0;
        virtual unsigned int GetCount() const       = 0;
        virtual RenderingID  GetRenderingID() const = 0;

        static Common::Memory::Shared<IndexBuffer> Create( const void* data, uint32_t size,
                                                   OpenGLBufferUsage usage = OpenGLBufferUsage::Static );
        static Common::Memory::Shared<IndexBuffer> Create( uint32_t          size,
                                                   OpenGLBufferUsage usage = OpenGLBufferUsage::Dynamic );
    };
} // namespace Radiant