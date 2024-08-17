#pragma once

namespace Radiant
{
    class UniformBuffer : public Memory::RefCounted
    {
    public:
        virtual ~UniformBuffer() = default;

        virtual void SetData( const void* data, std::size_t size, uint32_t offset = 0 )    = 0;
        virtual void RT_SetData( const void* data, std::size_t size, uint32_t offset = 0 ) = 0;

        virtual uint32_t GetBinding() const = 0;

        static Memory::Shared<UniformBuffer> Create( uint32_t size, uint32_t binding );
    };
} // namespace Radiant