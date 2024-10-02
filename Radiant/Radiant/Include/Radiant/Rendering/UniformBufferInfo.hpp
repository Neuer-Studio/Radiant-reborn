#pragma once

#include <Radiant/Rendering/UniformBuffer.hpp>
#include "RenderingTypes.hpp"

namespace Radiant
{
    class UniformBufferInfo : public Common::Memory::RefCounted
    {
    public:
        ~UniformBufferInfo() = default;

        void Create( uint32_t size, BindingPoint binding );

        void Set( const Common::Memory::Shared<UniformBuffer>& uniformBuffer, BindingPoint binding );
        Common::Memory::Shared<UniformBuffer> Get( const BindingPoint binding );

    private:
        std::unordered_map<BindingPoint, Common::Memory::Shared<UniformBuffer>> m_UniformBuffers;
    };
} // namespace Radiant