#include <Radiant/Rendering/Material.hpp>
#include <Radiant/Rendering/Shader.hpp>
#include <Radiant/Core/Memory/Buffer.hpp>

namespace Radiant
{
	class OpenGLMaterial : public Material
	{
	public:
		OpenGLMaterial(const Memory::Shared<Shader>& shader);

		virtual const Memory::Shared<Shader>& GetShader() const override { return m_Shader; }

		virtual void Use() const override;

		static void SetUBOMember(BindingPoint binding, const std::string& memberName, const glm::vec3& value);
		static void SetUBOMember(BindingPoint binding, const std::string& memberName, const glm::vec2& value);
		static void SetUBOMember(BindingPoint binding, const std::string& memberName, const glm::mat4& value);
		static void SetUBOMember(BindingPoint binding, const std::string& memberName, float value);
		static void SetUBOMember(BindingPoint binding, const std::string& memberName, bool value);

		static void SetUBO(BindingPoint binding, const void* data, std::size_t size, std::size_t offset);

		virtual void SetImage2D(const TextureDescriptor& descriptor, const Memory::Shared<Texture2D>& texture2D) const override;
		virtual void SetImage2D(const TextureDescriptor& descriptor, const Memory::Shared<Image2D>& image2D) const override;

		virtual const SamplerUniform& GetSamplerInformation(const std::string& name) const override;

		virtual void UpdateForRendering() const override;
		virtual void SetMat4(const std::string& name, const glm::mat4& value, std::optional<uint32_t> arrayIndex) const override;
		virtual void SetBool(const std::string& name, bool value, std::optional<uint32_t> arrayIndex) const override;
		virtual void SetUint(const std::string& name, uint32_t value, std::optional<uint32_t> arrayIndex) const override;
		virtual void SetFloat(const std::string& name, float value, std::optional<uint32_t> arrayIndex) const override;
		virtual void SetVec3(const std::string& name, const glm::vec3 value, std::optional<uint32_t> arrayIndex) const override;
		virtual void SetVec4(const std::string& name, const glm::vec4 value, std::optional<uint32_t> arrayIndex) const override;
	private:
		void SetUniform(const std::string& name, RadiantShaderDataType type, const void* value, std::optional<uint32_t> arrayIndex) const;
	private:
		Memory::Shared<Shader> m_Shader;
		mutable Memory::Buffer m_BufferValues;
		mutable std::vector<Image2D> m_Images2D;
	};
}