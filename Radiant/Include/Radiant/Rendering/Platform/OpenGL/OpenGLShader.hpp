#pragma once

#include <Radiant/Rendering/Shader.hpp>
#include <Rendering/RenderingTypes.hpp>

#include <spirv_cross/spirv_glsl.hpp>

namespace Radiant
{
	class OpenGLShader : public Shader
	{
	public:
		OpenGLShader(const std::filesystem::path& path);
		virtual ~OpenGLShader() override = default;

		virtual void Reload() override;
		virtual void Use() const override;

		virtual void SetUniform(RadiantShaderType shaderType, BindingPoint point, const std::string& name, const glm::vec3& value) const override;
		virtual void SetUniform(RadiantShaderType shaderType, BindingPoint point, const std::string& name, const glm::vec2& value) const override;
		virtual void SetUniform(RadiantShaderType shaderType, BindingPoint point, const std::string& name, float value) const override;
		virtual void SetUniform(RadiantShaderType shaderType, BindingPoint point, const std::string& name, bool value) const override;
	private:
	private:
		void Load(const std::string& shader—ontent);
		void ParseBuffers(RadiantShaderType type, const std::vector<uint32_t>& data);
		void CompileToSPIR_V();
		void Upload();
		void PreProcess(const std::string& content);
	private:
		GLuint OGLGetUniformPosition(const std::string& name);
	private:
		std::string m_Name;
		std::filesystem::path m_FilePath;
		RenderingID m_RenderingID = 0;

		std::unordered_map<RadiantShaderType, std::string> m_ShaderSource;
		std::unordered_map<RadiantShaderType, std::vector<uint32_t>> m_ShaderBinary;
		std::unordered_map<std::string, SamplerUniform> m_Resources;

		inline static std::unordered_map<RadiantShaderType, std::unordered_map<BindingPoint, ShaderUniformBuffer>> s_UniformBuffers;
	};
}