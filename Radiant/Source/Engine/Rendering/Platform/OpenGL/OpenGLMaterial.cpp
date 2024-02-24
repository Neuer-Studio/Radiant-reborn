#include <glad/glad.h>

#include <Radiant/Rendering/Platform/OpenGL/OpenGLMaterial.hpp>
#include <Radiant/Rendering/Platform/OpenGL/OpenGLShader.hpp>
#include <Radiant/Rendering/Platform/OpenGL/OpenGLImage.hpp>
#include <Radiant/Rendering/Rendering.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Radiant
{

	OpenGLMaterial::OpenGLMaterial(const Memory::Shared<Shader>& shader)
		: m_Shader(shader)
	{

		m_BufferValues.Allocate(m_Shader.As<OpenGLShader>()->m_UniformTotalOffset); 
		m_BufferValues.ZeroInitialize();
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

	void OpenGLMaterial::SetImage2D(const std::string& name, const Memory::Shared<Texture2D>& texture2D) const
	{
		Memory::Shared<const OpenGLMaterial> instance(this);
		Rendering::SubmitCommand([name, instance, texture2D]() mutable
			{
				auto samplerBuffer = instance->m_Shader.As<OpenGLShader>()->m_Resources[name];
				glBindSampler(samplerBuffer.Binding, texture2D->GetImage2D().As<OpenGLImage2D>()->GetSamplerID());
				glBindTextureUnit(samplerBuffer.Binding, texture2D->GetImage2D()->GetTextureID());
			});
	}

	void OpenGLMaterial::SetImage2D(const std::string& name, const Memory::Shared<Image2D>& image2D) const
	{
		Memory::Shared<const OpenGLMaterial> instance(this);
		Rendering::SubmitCommand([name, instance, image2D]() mutable
			{
				if (!image2D)
					return;

				auto samplerBuffer = instance->m_Shader.As<OpenGLShader>()->m_Resources[name];
				glBindSampler(samplerBuffer.Binding, image2D.As<OpenGLImage2D>()->GetSamplerID());
				glBindTextureUnit(samplerBuffer.Binding, image2D->GetTextureID());
			});
	}

	void OpenGLMaterial::LoadUniformToBuffer(const std::string& name, RadiantShaderType type, RadiantShaderDataType dataType) const
	{
		
	}

	void OpenGLMaterial::SetMat4(const std::string& name, const glm::mat4& value) const
	{
		if (m_Shader.As<OpenGLShader>()->m_Uniforms.find(name) == m_Shader.As<OpenGLShader>()->m_Uniforms.end())
			RADIANT_VERIFY(false);

		Memory::Shared<const OpenGLMaterial> instance(this);
		Rendering::SubmitCommand([name, instance, value]() mutable
			{
				const auto& buffer = instance->m_Shader.As<OpenGLShader>()->m_Uniforms[name];
				RADIANT_VERIFY(buffer.Position != (uint32_t)-1);

				instance->m_BufferValues.Write((void*)&value, buffer.Size, buffer.totalOffset);

				glUseProgram(instance->m_Shader.As<OpenGLShader>()->m_RenderingID);
				glUniformMatrix4fv(buffer.Position, 1, GL_FALSE, glm::value_ptr(value));
				glUseProgram(0);
			});
	}

	void OpenGLMaterial::SetBool(const std::string& name, bool value) const
	{
		if (m_Shader.As<OpenGLShader>()->m_Uniforms.find(name) == m_Shader.As<OpenGLShader>()->m_Uniforms.end())
			RADIANT_VERIFY(false);

		Memory::Shared<const OpenGLMaterial> instance(this);
		Rendering::SubmitCommand([name, instance, value]() mutable
			{
				const auto& buffer = instance->m_Shader.As<OpenGLShader>()->m_Uniforms[name];
				RADIANT_VERIFY(buffer.Position != (uint32_t)-1);

				instance->m_BufferValues.Write((void*)&value, buffer.Size, buffer.totalOffset);

				glUseProgram(instance->m_Shader.As<OpenGLShader>()->m_RenderingID);
				glUniform1i(buffer.Position, value ? 1 : 0);
				glUseProgram(0);
			});
	}

}