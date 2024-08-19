#include <Radiant/Rendering/UniformBufferInfo.hpp>

namespace Radiant
{

    void UniformBufferInfo::Create( uint32_t size, BindingPoint binding )
    {
        const auto uniformBuffer = UniformBuffer::Create(size, binding);
        Set(uniformBuffer, binding);
    }

    void UniformBufferInfo::Set( const Memory::Shared<UniformBuffer>& uniformBuffer, BindingPoint binding )
    {
        m_UniformBuffers[binding] = uniformBuffer;
    }

    Radiant::Memory::Shared<Radiant::UniformBuffer> UniformBufferInfo::Get( const BindingPoint binding )
    {
        return m_UniformBuffers[binding];
    }

} // namespace Radiant