#include <glad/glad.h>

#include <Radiant/Rendering/Platform/OpenGL/OpenGLMaterial.hpp>
#include <Radiant/Rendering/Platform/OpenGL/OpenGLShader.hpp>
#include <Radiant/Rendering/Platform/OpenGL/OpenGLImage.hpp>
#include <Radiant/Rendering/Rendering.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Radiant
{
    OpenGLMaterial::OpenGLMaterial( const Memory::Shared<Shader>& shader ) : m_Shader( shader )
    {
    }

    void OpenGLMaterial::Use() const
    {
        m_Shader->Use();
    }

    void OpenGLMaterial::UpdateImages() const
    {
        // Update Image2D. DI means DescriptorAndImage
        Memory::Shared<const OpenGLMaterial> instance( this );
        Rendering::SubmitCommand(
             [instance]() mutable
             {
                 for ( const auto& DI : instance->m_Images2D )
                 {
                     const auto& image2D = DI.second.second;
                     if ( !image2D )
                     {
                         continue;
                     }
                     const auto& descriptor = DI.second.first;
                     const auto& samplerBuffer =
                          instance->m_Shader.As<OpenGLShader>()->m_Resources[descriptor.Name];
                     if ( descriptor.ArrayIndex.has_value() )
                     {
                         RADIANT_VERIFY( samplerBuffer.Uniform.ArraySize > 1 ); // NOTE: texture must be arrayed
                     }

                     const auto binding = samplerBuffer.Uniform.Binding +
                                          descriptor.ArrayIndex.value_or( 0 ); // NOTE: Using for array textures

                     glBindTextureUnit( binding, image2D->GetTextureID() );
                     glBindSampler( binding,
                                    descriptor.Sampler.value_or( image2D.As<OpenGLImage2D>()->GetSamplerID() ) );
                 }
             } );
    }

    void OpenGLMaterial::UpdateUnifroms() const
    {
        Memory::Shared<const OpenGLMaterial> instance( this );
        Rendering::SubmitCommand(
             [instance]() mutable
             {
                 for ( const auto& uniform : instance->m_Uniforms )
                 {
                     const auto& name       = uniform.second.Name;
                     const auto& arrayIndex = uniform.second.ArrayIndex;
                     const auto& type       = uniform.second.Type;
                     const auto& value      = uniform.second.Value;

                     if ( instance->m_Shader.As<OpenGLShader>()->m_Uniforms.find( name ) ==
                          instance->m_Shader.As<OpenGLShader>()->m_Uniforms.end() )
                         RADIANT_VERIFY( false );

                     const auto& buffer = instance->m_Shader.As<OpenGLShader>()->m_Uniforms[name];
                     if ( arrayIndex.has_value() )
                     {
                         RADIANT_VERIFY( buffer.Uniform.ArraySize > 1 ); // NOTE: texture must be arrayed
                     }
                     const auto binding = buffer.Uniform.Binding + arrayIndex.value_or( 0 );

                     glUseProgram( instance->m_Shader.As<OpenGLShader>()->m_RenderingID );
                     switch ( type )
                     {
                         case RadiantShaderDataType::Float:
                             glUniform1f( binding, std::any_cast<float>( value ) );
                             break;
                         case RadiantShaderDataType::Int:
                             glUniform1i( binding, std::any_cast<int>( value ) );
                             break;
                         case RadiantShaderDataType::Bool:
                             glUniform1i( binding, std::any_cast<bool>( value ) ? 1 : 0 );
                             break;
                         case RadiantShaderDataType::UInt:
                             glUniform1ui( binding, std::any_cast<uint32_t>( value ) );
                             break;
                         case RadiantShaderDataType::Float3:
                         {
                             glm::vec3 vecValue = std::any_cast<glm::vec3>( value );
                             glUniform3f( binding, vecValue.x, vecValue.y, vecValue.z );
                             break;
                         }
                         case RadiantShaderDataType::Float4:
                         {
                             glm::vec4 vecValue = std::any_cast<glm::vec4>( value );
                             glUniform4f( binding, vecValue.x, vecValue.y, vecValue.z, vecValue.w );
                             break;
                         }
                         case RadiantShaderDataType::Mat4:
                         {
                             glm::mat4 newValue = std::any_cast<glm::mat4>( value );
                             glUniformMatrix4fv( binding, 1, GL_FALSE, glm::value_ptr( newValue ) );
                             break;
                         }
                         default:
                             RADIANT_VERIFY( false, "Unsupported type" ); // Unsupported type
                     }
                     glUseProgram( 0 );
                 }
             } );
    }

    void OpenGLMaterial::RT_UpdateForRendering() const
    {
        UpdateImages();
        UpdateUnifroms();
    }

    void OpenGLMaterial::SetImage2D( const TextureDescriptor&       descriptor,
                                     const Memory::Shared<Image2D>& image2D ) const
    {

        if ( !image2D )
            return;
        m_Images2D[descriptor.Name] = std::make_pair( descriptor, image2D );
#if 0
		Memory::Shared<const OpenGLMaterial> instance(this);
		Rendering::SubmitCommand([descriptor, instance, image2D]() mutable
			{
				if (!image2D)
					return;

				const auto& samplerBuffer = instance->m_Shader.As<OpenGLShader>()->m_Resources[descriptor.Name];	
				if (descriptor.ArrayIndex.has_value())
				{
					RADIANT_VERIFY(samplerBuffer.Uniform.ArraySize > 1); //NOTE: texture must be arrayed
				}

				const auto binding = samplerBuffer.Uniform.Binding + descriptor.ArrayIndex.value_or(0);// NOTE: Using for array textures

				glBindTextureUnit(binding, image2D->GetTextureID());
				glBindSampler(binding, descriptor.Sampler.value_or(image2D.As<OpenGLImage2D>()->GetSamplerID()));
			});
#endif
    }

    void OpenGLMaterial::SetImage2D( const TextureDescriptor&         descriptor,
                                     const Memory::Shared<Texture2D>& texture2D ) const
    {
        if ( !texture2D )
            return;
        SetImage2D( descriptor, texture2D->GetImage2D() );
    }

    const Radiant::SamplerUniform& OpenGLMaterial::GetSamplerInformation( const std::string& name ) const
    {
        const auto& buffer = m_Shader.As<OpenGLShader>()->m_Resources[name];

        return buffer;
    }

    void OpenGLMaterial::SetMat4( const std::string& name, const glm::mat4& value,
                                  std::optional<uint32_t> arrayIndex ) const
    {
        const std::string _name = ( !arrayIndex ) ? name : ( name + "[" + std::to_string( *arrayIndex ) + "]" );
        m_Uniforms[_name]       = { name, value, arrayIndex, RadiantShaderDataType::Mat4 };
        /*SetUniform( name, RadiantShaderDataType::Mat4, &value, arrayIndex );*/
    }

    void OpenGLMaterial::SetBool( const std::string& name, bool value, std::optional<uint32_t> arrayIndex ) const
    {
        m_Uniforms[name] = { name, value, arrayIndex, RadiantShaderDataType::Bool };
        // SetUniform( name, RadiantShaderDataType::Bool, &value, arrayIndex );
    }

    void OpenGLMaterial::SetUint( const std::string& name, uint32_t value,
                                  std::optional<uint32_t> arrayIndex ) const
    {
        const std::string _name = ( !arrayIndex ) ? name : ( name + "[" + std::to_string( *arrayIndex ) + "]" );
        m_Uniforms[_name]       = { name, value, arrayIndex, RadiantShaderDataType::UInt };
        // SetUniform( name, RadiantShaderDataType::UInt, &value, arrayIndex );
    }

    void OpenGLMaterial::SetFloat( const std::string& name, float value, std::optional<uint32_t> arrayIndex ) const
    {
        m_Uniforms[name] = { name, value, arrayIndex, RadiantShaderDataType::Float };
        // SetUniform( name, RadiantShaderDataType::Float, &value, arrayIndex );
    }

    void OpenGLMaterial::SetVec3( const std::string& name, const glm::vec3 value,
                                  std::optional<uint32_t> arrayIndex ) const
    {
        m_Uniforms[name] = { name, value, arrayIndex, RadiantShaderDataType::Float3 };
        // SetUniform( name, RadiantShaderDataType::Float3, &value, arrayIndex );
    }

    void OpenGLMaterial::SetVec4( const std::string& name, const glm::vec4 value,
                                  std::optional<uint32_t> arrayIndex ) const
    {
        m_Uniforms[name] = { name, value, arrayIndex, RadiantShaderDataType::Float4 };
        // SetUniform( name, RadiantShaderDataType::Float4, &value, arrayIndex );
    }

    void OpenGLMaterial::SetUBOMember( BindingPoint binding, const std::string& memberName,
                                       const glm::vec3& value )
    {
        Rendering::SubmitCommand(
             [binding, memberName, value]() mutable
             {
                 ShaderUniformBufferObject buffer  = OpenGLShader::s_UniformBuffers[binding];
                 UBOField uniform = buffer.Uniforms[memberName];
                 RADIANT_VERIFY( uniform.Name != "" );

                 if ( buffer.Name.empty() )
                     RA_WARN( "[ OpenGLMaterial::SetUniform ] bufferName is empty" );

                 glBindBuffer( GL_UNIFORM_BUFFER, buffer.RenderingID );
                 glBufferSubData( GL_UNIFORM_BUFFER, uniform.Offset, uniform.Size, &value[0] );
                 glBindBuffer( GL_UNIFORM_BUFFER, 0 );
             } );
    }

    void OpenGLMaterial::SetUBOMember( BindingPoint binding, const std::string& memberName,
                                       const glm::vec2& value )
    {
        Rendering::SubmitCommand(
             [binding, memberName, value]() mutable
             {
                 ShaderUniformBufferObject buffer  = OpenGLShader::s_UniformBuffers[binding];
                 UBOField uniform = buffer.Uniforms[memberName];
                 RADIANT_VERIFY( uniform.Name != "" );

                 if ( buffer.Name.empty() )
                     RA_WARN( "[ OpenGLMaterial::SetUniform ] bufferName is empty" );

                 glBindBuffer( GL_UNIFORM_BUFFER, buffer.RenderingID );
                 glBufferSubData( GL_UNIFORM_BUFFER, uniform.Offset, uniform.Size, &value[0] );
                 glBindBuffer( GL_UNIFORM_BUFFER, 0 );
             } );
    }

    void OpenGLMaterial::SetUBOMember( BindingPoint binding, const std::string& memberName,
                                       const glm::mat4& value )
    {
        Rendering::SubmitCommand(
             [binding, memberName, value]() mutable
             {
                 ShaderUniformBufferObject buffer  = OpenGLShader::s_UniformBuffers[binding];
                 UBOField uniform = buffer.Uniforms[memberName];
                 RADIANT_VERIFY( uniform.Name != "" );

                 if ( buffer.Name.empty() )
                     RA_WARN( "[ OpenGLMaterial::SetUniform ] bufferName is empty" );

                 glBindBuffer( GL_UNIFORM_BUFFER, buffer.RenderingID );
                 glBufferSubData( GL_UNIFORM_BUFFER, uniform.Offset, uniform.Size, glm::value_ptr( value ) );
                 glBindBuffer( GL_UNIFORM_BUFFER, 0 );
             } );
    }

    void OpenGLMaterial::SetUBOMember( BindingPoint binding, const std::string& memberName, float value )
    {
        Rendering::SubmitCommand(
             [binding, memberName, value]() mutable
             {
                 ShaderUniformBufferObject buffer  = OpenGLShader::s_UniformBuffers[binding];
                 UBOField uniform = buffer.Uniforms[memberName];
                 RADIANT_VERIFY( uniform.Name != "" );

                 if ( buffer.Name.empty() )
                     RA_WARN( "[ OpenGLMaterial::SetUniform ] bufferName is empty" );

                 glBindBuffer( GL_UNIFORM_BUFFER, buffer.RenderingID );
                 glBufferSubData( GL_UNIFORM_BUFFER, uniform.Offset, uniform.Size, &value );
                 glBindBuffer( GL_UNIFORM_BUFFER, 0 );
             } );
    }

    void OpenGLMaterial::SetUBOMember( BindingPoint binding, const std::string& memberName, bool value )
    {
        Rendering::SubmitCommand(
             [binding, memberName, value]() mutable
             {
                 ShaderUniformBufferObject buffer  = OpenGLShader::s_UniformBuffers[binding];
                 UBOField uniform = buffer.Uniforms[memberName];
                 RADIANT_VERIFY( uniform.Name != "" );

                 if ( buffer.Name.empty() )
                     RA_WARN( "[ OpenGLMaterial::SetUniform ] bufferName is empty" );

                 glBindBuffer( GL_UNIFORM_BUFFER, buffer.RenderingID );
                 glBufferSubData( GL_UNIFORM_BUFFER, uniform.Offset, uniform.Size, &value );
                 glBindBuffer( GL_UNIFORM_BUFFER, 0 );

                 // glNamedBufferSubData
             } );
    }

    void OpenGLMaterial::SetUBO( BindingPoint binding, const void* data, std::size_t size, std::size_t offset )
    {
        Memory::Buffer storageBuffer = Memory::Buffer::Copy( data, size );
        Rendering::SubmitCommand(
             [binding, offset, storageBuffer]() mutable
             {
                 ShaderUniformBufferObject ubuffer = OpenGLShader::s_UniformBuffers[binding];
                 RADIANT_VERIFY( ubuffer.Name != "" );
                 RADIANT_VERIFY( ubuffer.Size == storageBuffer.Size );

                 if ( ubuffer.Name.empty() )
                     RA_WARN( "[ OpenGLMaterial::SetUniform ] bufferName is empty" );

                 glBindBuffer( GL_UNIFORM_BUFFER, ubuffer.RenderingID );
                 glBufferSubData( GL_UNIFORM_BUFFER, offset, storageBuffer.Size, storageBuffer.Data );
                 glBindBuffer( GL_UNIFORM_BUFFER, 0 );

                 storageBuffer.Release();
                 // glNamedBufferSubData
             } );
    }
} // namespace Radiant