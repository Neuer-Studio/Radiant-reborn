#pragma once

#include <Radiant/Rendering/UniformBuffer.hpp>
#include "RenderingTypes.hpp"

namespace Radiant
{
    class UniformBufferInfo : public Memory::RefCounted
    {
    public:
        ~UniformBufferInfo() = default;

        void Create( uint32_t size, BindingPoint binding );

        void Set( const Memory::Shared<UniformBuffer>& uniformBuffer, BindingPoint binding );
        Memory::Shared<UniformBuffer> Get( const BindingPoint binding );

    private:
        std::unordered_map<BindingPoint, Memory::Shared<UniformBuffer>> m_UniformBuffers;
    };
} // namespace Radiant