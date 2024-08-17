#include <glad/glad.h>

#include <Radiant/Rendering/Platform/OpenGL/OpenGLUniformBuffer.hpp>
#include <Radiant/Rendering/Platform/OpenGL/OpenGLShader.hpp>
#include <Radiant/Rendering/Rendering.hpp>

namespace Radiant
{

    OpenGLUniformBuffer::OpenGLUniformBuffer( uint32_t size, uint32_t binding )
         : m_Size( size ), m_Binding( binding )
    {
        Memory::Shared<OpenGLUniformBuffer> instance = this;
        Rendering::SubmitCommand( [instance]() mutable { instance->RT_Invalidate(); } );
    }

    void OpenGLUniformBuffer::SetData( const void* data, std::size_t size, uint32_t offset /*= 0 */ )
    {
        m_LocalStorage                               = Memory::Buffer::Copy( data, size );
        Memory::Shared<OpenGLUniformBuffer> instance = this;
        Rendering::SubmitCommand( [instance, size, offset]() mutable
                                  { instance->RT_SetData( instance->m_LocalStorage.Data, size, offset ); } );
    }

    void OpenGLUniformBuffer::RT_SetData( const void* data, std::size_t size, uint32_t offset /*= 0 */ )
    {
        glBindBuffer( GL_UNIFORM_BUFFER, *m_UBORenderingID );
        glBufferSubData( GL_UNIFORM_BUFFER, offset, size, data );
        glBindBuffer( GL_UNIFORM_BUFFER, 0 );
    }

    void OpenGLUniformBuffer::RT_Invalidate()
    {
        m_UBORenderingID = OpenGLShader::GetUniformBuffers().at( m_Binding ).RenderingID;
        RADIANT_VERIFY( m_UBORenderingID );
    }

} // namespace Radiant