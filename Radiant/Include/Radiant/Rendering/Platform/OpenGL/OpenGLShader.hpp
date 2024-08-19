#pragma once

#include <Radiant/Rendering/Shader.hpp>
#include <Rendering/RenderingTypes.hpp>

#include <spirv_cross/spirv_glsl.hpp>

namespace Radiant
{
    class OpenGLShader : public Shader
    {
    public:
        OpenGLShader( const std::filesystem::path& path );
        virtual ~OpenGLShader() override = default;

        virtual void Reload() override;
        virtual void Use( BindUsage use = BindUsage::Bind ) const override;
        virtual void RT_Use( BindUsage use = BindUsage::Bind ) const override;


        virtual RenderingID GetRenderingID() const
        {
            return m_RenderingID;
        }

        virtual const std::string GetShaderName() const
        {
            return m_Name;
        }

        static const auto& GetUniformBuffers() { return s_UniformBuffers; }

        virtual inline UBOField GetUniformBufferField( const BindingPoint       binding,
                                                       const std::string& fieldName ) const override
        {
            return s_UniformBuffers[binding].Uniforms[fieldName];
        }

    private:
    private:
        void Load( const std::string& shader—ontent );
        void ParseShaderBuffers( RadiantShaderType type, const std::vector<uint32_t>& data );
        void ApplyShaderBuffers();
        void ParseConstantBuffers( RadiantShaderType type, const std::vector<uint32_t>& data );
        [[nodiscard]] const std::unordered_map<RadiantShaderType, std::vector<uint32_t>>
        CompileToSPIR_V( const std::unordered_map<RadiantShaderType, std::string>& shaderSource );
        [[nodiscard]] std::unordered_map<RadiantShaderType, std::vector<uint32_t>>
             UploadFromBinaryFile( const std::filesystem::path& path );
        void Upload( const std::unordered_map<RadiantShaderType, std::vector<uint32_t>>& shaderBinary );
        void FillBinaryFile( const std::filesystem::path& path, const std::filesystem::path& binaryPath,
                             const std::vector<uint32_t>& binary );
        [[nodiscard]] const std::unordered_map<RadiantShaderType, std::string>
        PreProcess( const std::string& content );

        void UploadUniformMat4( int32_t location, const glm::mat4& values );

    private:
        GLuint                OGLGetUniformPosition( const std::string& name );
        void                  CreateDirectoriesIfNotFound();
        std::vector<uint32_t> TryToLoadCachedData( const std::filesystem::path& path, RadiantShaderType type );
        [[nodiscard]] const std::vector<RadiantShaderType>
        ListShaderTypesByFileName( const std::string& fileName ) const;

    private:
        uint32_t              m_UniformTotalOffset = 0;
        std::string           m_Name;
        std::filesystem::path m_FilePath;
        RenderingID           m_RenderingID = 0;

        std::unordered_map<std::string, SamplerUniform> m_Resources;

        inline static std::unordered_map<BindingPoint, ShaderUniformBufferObject> s_UniformBuffers; // UBOs
        std::unordered_map<std::string, Uniform>                                  m_Uniforms; // plain uniforms

    private:
        friend class OpenGLMaterial;
    };
} // namespace Radiant