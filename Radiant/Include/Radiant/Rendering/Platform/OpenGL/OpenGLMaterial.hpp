#include <Radiant/Rendering/Material.hpp>
#include <Radiant/Core/Memory/Buffer.hpp>

namespace Radiant
{
	class OpenGLMaterial : public Material
	{
	public:
		OpenGLMaterial(const Memory::Shared<Shader>& shader);

		virtual void Use() const override {}

		virtual void SetUniform(RadiantShaderType shaderType, BindingPoint point, const std::string& name, const glm::vec3& value) const override;
		virtual void SetUniform(RadiantShaderType shaderType, BindingPoint point, const std::string& name, const glm::vec2& value) const override;
		virtual void SetUniform(RadiantShaderType shaderType, BindingPoint point, const std::string& name, float value) const override;
		virtual void SetUniform(RadiantShaderType shaderType, BindingPoint point, const std::string& name, bool value) const override;
		virtual void SetUniform(const std::string& name, const Memory::Shared<Texture2D>& texture2D) const override;
	private:
		Memory::Shared<Shader> m_Shader;
		Memory::Buffer m_BufferValues;
	};
}