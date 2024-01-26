#include <glad/glad.h>

#include <Radiant/Rendering/Platform/OpenGL/OpenGLMaterial.hpp>
#include <Radiant/Rendering/Platform/OpenGL/OpenGLShader.hpp>
#include <Radiant/Rendering/Rendering.hpp>

namespace Radiant
{

	OpenGLMaterial::OpenGLMaterial(const Memory::Shared<Shader>& shader)
		: m_Shader(shader)
	{
		//m_BufferValues.Allocate()
	}

	void OpenGLMaterial::SetUniform(RadiantShaderType shaderType, BindingPoint point, const std::string& name, const glm::vec3& value) const
	{
		Memory::Shared<const OpenGLMaterial> instance(this);
		Rendering::SubmitCommand([shaderType, point, name, value, instance]() mutable
			{
				ShaderUniformBuffer buffer = instance->m_Shader.As<OpenGLShader>()->m_UniformBuffers[shaderType][point];
				MemberUniformBuffer uniform = buffer.Uniforms[name];

				glBindBuffer(GL_UNIFORM_BUFFER, buffer.RenderingID);
				glBufferSubData(GL_UNIFORM_BUFFER, uniform.Offset, uniform.Size, &value[0]);
				glBindBuffer(GL_UNIFORM_BUFFER, 0);
			});
	}

	void OpenGLMaterial::SetUniform(RadiantShaderType shaderType, BindingPoint point, const std::string& name, const glm::vec2& value) const
	{
		Memory::Shared<const OpenGLMaterial> instance(this);
		Rendering::SubmitCommand([shaderType, point, name, value, instance]() mutable
			{
				ShaderUniformBuffer buffer = instance->m_Shader.As<OpenGLShader>()->m_UniformBuffers[shaderType][point];
				MemberUniformBuffer uniform = buffer.Uniforms[name];

				glBindBuffer(GL_UNIFORM_BUFFER, buffer.RenderingID);
				glBufferSubData(GL_UNIFORM_BUFFER, uniform.Offset, uniform.Size, &value[0]);
				glBindBuffer(GL_UNIFORM_BUFFER, 0);
			});
	}

	void OpenGLMaterial::SetUniform(RadiantShaderType shaderType, BindingPoint point, const std::string& name, float value) const
	{
		Memory::Shared<const OpenGLMaterial> instance(this);
		Rendering::SubmitCommand([shaderType, point, name, value, instance]() mutable
			{
				ShaderUniformBuffer buffer = instance->m_Shader.As<OpenGLShader>()->m_UniformBuffers[shaderType][point];
				MemberUniformBuffer uniform = buffer.Uniforms[name];

				glBindBuffer(GL_UNIFORM_BUFFER, buffer.RenderingID);
				glBufferSubData(GL_UNIFORM_BUFFER, uniform.Offset, uniform.Size, &value);
				glBindBuffer(GL_UNIFORM_BUFFER, 0);
			});
	}

	void OpenGLMaterial::SetUniform(RadiantShaderType shaderType, BindingPoint point, const std::string& name, bool value) const
	{
		Memory::Shared<const OpenGLMaterial> instance(this);
		Rendering::SubmitCommand([shaderType, point, name, value, instance]() mutable
			{
				ShaderUniformBuffer buffer = instance->m_Shader.As<OpenGLShader>()->m_UniformBuffers[shaderType][point];
				MemberUniformBuffer uniform = buffer.Uniforms[name];

				glBindBuffer(GL_UNIFORM_BUFFER, buffer.RenderingID);
				glBufferSubData(GL_UNIFORM_BUFFER, uniform.Offset, uniform.Size, &value);
				glBindBuffer(GL_UNIFORM_BUFFER, 0);

				// glNamedBufferSubData
			});
	}

	void OpenGLMaterial::SetUniform(const std::string& name, const Memory::Shared<Texture2D>& texture2D) const
	{
		Memory::Shared<const OpenGLMaterial> instance(this);
		Rendering::SubmitCommand([name, instance, texture2D]() mutable
			{
				auto samplerBuffer = instance->m_Shader.As<OpenGLShader>()->m_Resources[name];
				glBindTextureUnit(samplerBuffer.Binding, texture2D->GetImage2D()->GetTextureID());
			});
	}

}