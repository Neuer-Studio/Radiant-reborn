#include <glad/glad.h>

#include <Radiant/Rendering/Platform/OpenGL/OpenGLMaterial.hpp>
#include <Radiant/Rendering/Platform/OpenGL/OpenGLShader.hpp>
#include <Radiant/Rendering/Rendering.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Radiant
{

	OpenGLMaterial::OpenGLMaterial(const Memory::Shared<Shader>& shader)
		: m_Shader(shader)
	{
		//m_BufferValues.Allocate()
	}

	void OpenGLMaterial::Use() const
	{
		m_Shader->Use();
	}

	void OpenGLMaterial::SetUBO(BindingPoint binding, const std::string& name, const glm::vec3& value) const
	{
		Memory::Shared<const OpenGLMaterial> instance(this);
		Rendering::SubmitCommand([binding,  name, value, instance]() mutable
			{
				ShaderUniformBufferObject buffer = instance->m_Shader.As<OpenGLShader>()->m_UniformBuffers[binding];
				MemberUniformBufferObject uniform = buffer.Uniforms[name];
				RADIANT_VERIFY(uniform.Name != "");

				if (buffer.Name.empty())
					RA_WARN("[ OpenGLMaterial::SetUniform ] bufferName is empty");

				glBindBuffer(GL_UNIFORM_BUFFER, buffer.RenderingID);
				glBufferSubData(GL_UNIFORM_BUFFER, uniform.Offset, uniform.Size, &value[0]);
				glBindBuffer(GL_UNIFORM_BUFFER, 0);
			});
	}

	void OpenGLMaterial::SetUBO(BindingPoint binding, const std::string& name, const glm::vec2& value) const
	{
		Memory::Shared<const OpenGLMaterial> instance(this);
		Rendering::SubmitCommand([binding,  name, value, instance]() mutable
			{
				ShaderUniformBufferObject buffer = instance->m_Shader.As<OpenGLShader>()->m_UniformBuffers[binding];
				MemberUniformBufferObject uniform = buffer.Uniforms[name];
				RADIANT_VERIFY(uniform.Name != "");

				if (buffer.Name.empty())
					RA_WARN("[ OpenGLMaterial::SetUniform ] bufferName is empty");

				glBindBuffer(GL_UNIFORM_BUFFER, buffer.RenderingID);
				glBufferSubData(GL_UNIFORM_BUFFER, uniform.Offset, uniform.Size, &value[0]);
				glBindBuffer(GL_UNIFORM_BUFFER, 0);
			});
	}

	void OpenGLMaterial::SetUBO(BindingPoint binding, const std::string& name, const glm::mat4& value) const
	{
		Memory::Shared<const OpenGLMaterial> instance(this);
		Rendering::SubmitCommand([binding,  name, value, instance]() mutable
			{
				ShaderUniformBufferObject buffer = instance->m_Shader.As<OpenGLShader>()->m_UniformBuffers[binding];
				MemberUniformBufferObject uniform = buffer.Uniforms[name];
				RADIANT_VERIFY(uniform.Name != "");

				if (buffer.Name.empty())
					RA_WARN("[ OpenGLMaterial::SetUniform ] bufferName is empty");

				glBindBuffer(GL_UNIFORM_BUFFER, buffer.RenderingID);
				glBufferSubData(GL_UNIFORM_BUFFER, uniform.Offset, uniform.Size, glm::value_ptr(value));
				glBindBuffer(GL_UNIFORM_BUFFER, 0);
			});
	}

	void OpenGLMaterial::SetUBO(BindingPoint binding, const std::string& name, float value) const
	{
		Memory::Shared<const OpenGLMaterial> instance(this);
		Rendering::SubmitCommand([binding,  name, value, instance]() mutable
			{
				ShaderUniformBufferObject buffer = instance->m_Shader.As<OpenGLShader>()->m_UniformBuffers[binding];
				MemberUniformBufferObject uniform = buffer.Uniforms[name];
				RADIANT_VERIFY(uniform.Name != "");

				if (buffer.Name.empty())
					RA_WARN("[ OpenGLMaterial::SetUniform ] bufferName is empty");

				glBindBuffer(GL_UNIFORM_BUFFER, buffer.RenderingID);
				glBufferSubData(GL_UNIFORM_BUFFER, uniform.Offset, uniform.Size, &value);
				glBindBuffer(GL_UNIFORM_BUFFER, 0);
			});
	}

	void OpenGLMaterial::SetUBO(BindingPoint binding, const std::string& name, bool value) const
	{
		Memory::Shared<const OpenGLMaterial> instance(this);
		Rendering::SubmitCommand([binding,  name, value, instance]() mutable
			{
				ShaderUniformBufferObject buffer = instance->m_Shader.As<OpenGLShader>()->m_UniformBuffers[binding];
				MemberUniformBufferObject uniform = buffer.Uniforms[name];
				RADIANT_VERIFY(uniform.Name != "");

				if (buffer.Name.empty())
					RA_WARN("[ OpenGLMaterial::SetUniform ] bufferName is empty");

				glBindBuffer(GL_UNIFORM_BUFFER, buffer.RenderingID);
				glBufferSubData(GL_UNIFORM_BUFFER, uniform.Offset, uniform.Size, &value);
				glBindBuffer(GL_UNIFORM_BUFFER, 0);

				// glNamedBufferSubData
			});
	}

	void OpenGLMaterial::SetUBO(const std::string& name, const Memory::Shared<Texture2D>& texture2D) const
	{
		Memory::Shared<const OpenGLMaterial> instance(this);
		Rendering::SubmitCommand([name, instance, texture2D]() mutable
			{
				
				auto samplerBuffer = instance->m_Shader.As<OpenGLShader>()->m_Resources[name];
				glBindTextureUnit(samplerBuffer.Binding, texture2D->GetImage2D()->GetTextureID());
			});
	}

	void OpenGLMaterial::SetUBO(const std::string& name, const Memory::Shared<Image2D>& image2D) const
	{
		Memory::Shared<const OpenGLMaterial> instance(this);
		Rendering::SubmitCommand([name, instance, image2D]() mutable
			{
				if (!image2D)
					return;

				auto samplerBuffer = instance->m_Shader.As<OpenGLShader>()->m_Resources[name];
				glBindTextureUnit(samplerBuffer.Binding, image2D->GetTextureID());
			});
	}

	void OpenGLMaterial::LoadUniformToBuffer(const std::string& name, RadiantShaderType type, RadiantShaderDataType dataType) const
	{
		if (m_Uniforms.find(name) != m_Uniforms.end())
			RADIANT_VERIFY(false);
		Memory::Shared<const OpenGLMaterial> instance(this);
		Rendering::SubmitCommand([name, instance, type, dataType]() mutable
			{
				auto& buffer = instance->m_Uniforms[name];
				const auto pos = instance->m_Shader.As<OpenGLShader>()->OGLGetUniformPosition(name);
				buffer = {name, type, dataType, pos };
			});
	}

	void OpenGLMaterial::SetMat4(const std::string& name, const glm::mat4& value) const
	{
		if (m_Uniforms.find(name) == m_Uniforms.end())
			RADIANT_VERIFY(false);

		Memory::Shared<const OpenGLMaterial> instance(this);
		Rendering::SubmitCommand([name, instance, value]() mutable
			{
				const auto& buffer = instance->m_Uniforms[name];

				glUseProgram(instance->m_Shader.As<OpenGLShader>()->m_RenderingID);
				glUniformMatrix4fv(buffer.Position, 1, GL_FALSE, glm::value_ptr(value));
				glUseProgram(0);
			});
	}

}