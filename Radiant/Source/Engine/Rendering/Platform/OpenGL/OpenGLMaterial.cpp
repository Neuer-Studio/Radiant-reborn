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

    void OpenGLMaterial::UploadImageToShader( const ImageDescriptor&  imageDescriptor,
                                              Memory::Shared<Image2D> image2D ) const
    {
        if ( !image2D || !image2D->IsLoaded() )
        {
            return;
        }
        const auto& samplerBuffer = m_Shader.As<OpenGLShader>()->m_Resources[imageDescriptor.Name];
        if ( imageDescriptor.ArrayIndex.has_value() )
        {
            RADIANT_VERIFY( samplerBuffer.Uniform.ArraySize > 1 ); // NOTE: texture must be arrayed
        }

        const auto binding = samplerBuffer.Uniform.Binding +
                             imageDescriptor.ArrayIndex.value_or( 0 ); // NOTE: Using for array textures

        uint32_t id = image2D->GetTextureID();
        glBindSampler( binding, imageDescriptor.Sampler.value_or( image2D.As<OpenGLImage2D>()->GetSamplerID() ) );
        glBindTextureUnit( binding, id );
    }

    void OpenGLMaterial::UpdateImages() const
    {
        // Update Image2D. DI means DescriptorAndImage

        for ( const auto& DI : m_Images2D )
        {
            UploadImageToShader( DI.second.first, DI.second.second );
        }

        for ( const auto& DI : m_Textures2D )
        {
            UploadImageToShader( DI.second.first, DI.second.second->GetImage2D() );
        }
    }

    void OpenGLMaterial::UpdateUnifroms() const
    {
        for ( const auto& uniform : m_Uniforms )
        {
            const auto& name       = uniform.second.Name;
            const auto& arrayIndex = uniform.second.ArrayIndex;
            const auto& type       = uniform.second.Type;
            const auto& value      = uniform.second.Value;

            if ( m_Shader.As<OpenGLShader>()->m_Uniforms.find( name ) ==
                 m_Shader.As<OpenGLShader>()->m_Uniforms.end() )
                RADIANT_VERIFY( false );

            const auto& buffer = m_Shader.As<OpenGLShader>()->m_Uniforms[name];
            if ( arrayIndex.has_value() )
            {
                RADIANT_VERIFY( buffer.Uniform.ArraySize > 1 ); // NOTE: texture must be arrayed
            }
            const auto binding = buffer.Uniform.Binding + arrayIndex.value_or( 0 );

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
        }
    }

    void OpenGLMaterial::RT_UpdateForRendering() const
    {
        m_Shader->RT_Use( BindUsage::Bind );
        UpdateImages();
        UpdateUnifroms();
        m_Shader->RT_Use( BindUsage::Unbind );
    }

    void OpenGLMaterial::UpdateForRendering() const
    {
        Memory::Shared<const OpenGLMaterial> instance( this );
        Rendering::SubmitCommand( [instance]() mutable { instance->RT_UpdateForRendering(); } );
    }
    void OpenGLMaterial::SetImage2D( const ImageDescriptor&         descriptor,
                                     const Memory::Shared<Image2D>& image2D ) const
    {
        if ( !image2D )
            return;
        m_Images2D[descriptor.Name] = std::make_pair( descriptor, image2D );
    }

    void OpenGLMaterial::SetImage2D( const ImageDescriptor&           descriptor,
                                     const Memory::Shared<Texture2D>& texture2D ) const
    {
        if ( !texture2D )
            return;
        m_Textures2D[descriptor.Name] = std::make_pair( descriptor, texture2D );
    }

    void OpenGLMaterial::SetImage2D( const std::string&               uniformName,
                                     const Memory::Shared<Texture2D>& texture2D ) const
    {
        ImageDescriptor desc;
        desc.Name       = uniformName;
        desc.ArrayIndex = std::nullopt;
        desc.Sampler    = std::nullopt;

        SetImage2D( desc, texture2D );
    }

    void OpenGLMaterial::SetImage2D( const std::string& uniformName, const Memory::Shared<Image2D>& image2D ) const
    {
        ImageDescriptor desc;
        desc.Name       = uniformName;
        desc.ArrayIndex = std::nullopt;
        desc.Sampler    = std::nullopt;

        SetImage2D( desc, image2D );
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
        const std::string _name = ( !arrayIndex ) ? name : ( name + "[" + std::to_string( *arrayIndex ) + "]" );
        m_Uniforms[_name]       = { name, value, arrayIndex, RadiantShaderDataType::Bool };
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
        const std::string _name = ( !arrayIndex ) ? name : ( name + "[" + std::to_string( *arrayIndex ) + "]" );
        m_Uniforms[_name]       = { name, value, arrayIndex, RadiantShaderDataType::Float };
        // SetUniform( name, RadiantShaderDataType::Float, &value, arrayIndex );
    }

    void OpenGLMaterial::SetVec3( const std::string& name, const glm::vec3 value,
                                  std::optional<uint32_t> arrayIndex ) const
    {
        const std::string _name = ( !arrayIndex ) ? name : ( name + "[" + std::to_string( *arrayIndex ) + "]" );
        m_Uniforms[_name]       = { name, value, arrayIndex, RadiantShaderDataType::Float3 };
        // SetUniform( name, RadiantShaderDataType::Float3, &value, arrayIndex );
    }

    void OpenGLMaterial::SetVec4( const std::string& name, const glm::vec4 value,
                                  std::optional<uint32_t> arrayIndex ) const
    {
        const std::string _name = ( !arrayIndex ) ? name : ( name + "[" + std::to_string( *arrayIndex ) + "]" );
        m_Uniforms[_name]       = { name, value, arrayIndex, RadiantShaderDataType::Float4 };
        // SetUniform( name, RadiantShaderDataType::Float4, &value, arrayIndex );
    }

} // namespace Radiant