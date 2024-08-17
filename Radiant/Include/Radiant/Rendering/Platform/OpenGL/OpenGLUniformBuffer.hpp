#pragma once

#include <Radiant/Rendering/UniformBuffer.hpp>
#include <Rendering/RenderingTypes.hpp>
#include <Radiant/Core/Memory/Buffer.hpp>

namespace Radiant
{
    class OpenGLUniformBuffer : public UniformBuffer
    {
    public:
        OpenGLUniformBuffer( uint32_t size, uint32_t binding );
        virtual ~OpenGLUniformBuffer() override = default;

        virtual void SetData( const void* data, std::size_t size, uint32_t offset = 0 ) override;
        virtual void RT_SetData( const void* data, std::size_t size, uint32_t offset = 0 ) override;

        virtual uint32_t GetBinding() const override
        {
            return m_Binding;
        }
    private:
        uint32_t                    m_Size           = 0;
        BindingPoint                m_Binding        = 0;
        std::optional<BindingPoint> m_UBORenderingID;

        Memory::Buffer m_LocalStorage;
    private:
        void RT_Invalidate();
    };
} // namespace Radiant