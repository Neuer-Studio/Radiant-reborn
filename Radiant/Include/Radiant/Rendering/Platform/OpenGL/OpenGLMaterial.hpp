#include <Radiant/Rendering/Material.hpp>
#include <Radiant/Rendering/Shader.hpp>
#include <Radiant/Core/Memory/Buffer.hpp>

namespace Radiant
{
	class OpenGLMaterial : public Material
	{
	public:
		OpenGLMaterial(const Memory::Shared<Shader>& shader);

		virtual void Use() const override;

		virtual void SetUBO(BindingPoint binding, const std::string& name, const glm::vec3& value) const override;
		virtual void SetUBO(BindingPoint binding, const std::string& name, const glm::vec2& value) const override;
		virtual void SetUBO(BindingPoint binding, const std::string& name, const glm::mat4& value) const override;
		virtual void SetUBO(BindingPoint binding, const std::string& name, float value) const override;
		virtual void SetUBO(BindingPoint binding, const std::string& name, bool value) const override;
		virtual void SetUBO(const std::string& name, const Memory::Shared<Texture2D>& texture2D) const override;
		virtual void SetUBO(const std::string& name, const Memory::Shared<Image2D>& image2D) const override;

		virtual void LoadUniformToBuffer(const std::string& name, RadiantShaderType type, RadiantShaderDataType dataType) const override;
		virtual void SetMat4(const std::string& name, const glm::mat4& value) const override;

	private:
		Memory::Shared<Shader> m_Shader;
		Memory::Buffer m_BufferValues;

		mutable std::unordered_map<std::string, Uniform> m_Uniforms;
	};
}