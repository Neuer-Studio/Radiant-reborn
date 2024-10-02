#include <Radiant/Rendering/Material.hpp>
#include <Radiant/Rendering/Shader.hpp>
#include <Common/Core/Memory/Buffer.hpp>

#include <any>

namespace Radiant
{
    class OpenGLMaterial : public Material
    {
    public:
        OpenGLMaterial( const Common::Memory::Shared<Shader>& shader );

        virtual const Common::Memory::Shared<Shader>& GetShader() const override
        {
            return m_Shader;
        }

        virtual void Use() const override;

        virtual void SetImage2D( const ImageDescriptor&           descriptor,
                                 const Common::Memory::Shared<Texture2D>& texture2D ) const override;
        virtual void SetImage2D( const ImageDescriptor&         descriptor,
                                 const Common::Memory::Shared<Image2D>& image2D ) const override;

        virtual void SetImage2D( const std::string&               uniformName,
                                 const Common::Memory::Shared<Texture2D>& texture2D ) const override;
        virtual void SetImage2D( const std::string&             uniformName,
                                 const Common::Memory::Shared<Image2D>& image2D ) const override;

        virtual const SamplerUniform& GetSamplerInformation( const std::string& name ) const override;

        virtual void RT_UpdateForRendering() const override;
        virtual void UpdateForRendering() const override;

        virtual void SetMat4( const std::string& name, const glm::mat4& value,
                              std::optional<uint32_t> arrayIndex ) const override;
        virtual void SetBool( const std::string& name, bool value,
                              std::optional<uint32_t> arrayIndex ) const override;
        virtual void SetUint( const std::string& name, uint32_t value,
                              std::optional<uint32_t> arrayIndex ) const override;
        virtual void SetFloat( const std::string& name, float value,
                               std::optional<uint32_t> arrayIndex ) const override;
        virtual void SetVec3( const std::string& name, const glm::vec3 value,
                              std::optional<uint32_t> arrayIndex ) const override;
        virtual void SetVec4( const std::string& name, const glm::vec4 value,
                              std::optional<uint32_t> arrayIndex ) const override;

    private:
        void UpdateImages() const;
        void UpdateUnifroms() const;

    private:
        using DescriptorAndImage2D   = std::pair<ImageDescriptor, Common::Memory::Shared<Image2D>>;
        using DescriptorAndTexture2D = std::pair<ImageDescriptor, Common::Memory::Shared<Texture2D>>;

    private:
        void UploadImageToShader( const ImageDescriptor& imageDescriptor, Common::Memory::Shared<Image2D> image2D ) const;

    private:
        struct UniformParameters
        {
            std::string             Name;
            std::any                Value;
            std::optional<uint32_t> ArrayIndex;
            RadiantShaderDataType   Type;
        };

        Common::Memory::Shared<Shader>                                          m_Shader;
        mutable std::unordered_map<std::string, DescriptorAndImage2D>   m_Images2D;
        mutable std::unordered_map<std::string, DescriptorAndTexture2D> m_Textures2D;
        mutable std::unordered_map<std::string, UniformParameters>      m_Uniforms;
    };
} // namespace Radiant