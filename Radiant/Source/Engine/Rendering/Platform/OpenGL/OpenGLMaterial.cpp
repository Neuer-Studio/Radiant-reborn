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

	void OpenGLMaterial::SetUBO(BindingPoint binding, const std::string& name, const glm::vec3& value)
	{
		Rendering::SubmitCommand([binding,  name, value]() mutable
			{
				ShaderUniformBufferObject buffer = OpenGLShader::s_UniformBuffers[binding];
				MemberUniformBufferObject uniform = buffer.Uniforms[name];
				RADIANT_VERIFY(uniform.Name != "");

				if (buffer.Name.empty())
					RA_WARN("[ OpenGLMaterial::SetUniform ] bufferName is empty");

				glBindBuffer(GL_UNIFORM_BUFFER, buffer.RenderingID);
				glBufferSubData(GL_UNIFORM_BUFFER, uniform.Offset, uniform.Size, &value[0]);
				glBindBuffer(GL_UNIFORM_BUFFER, 0);
			});
	}

	void OpenGLMaterial::SetUBO(BindingPoint binding, const std::string& name, const glm::vec2& value)
	{
		Rendering::SubmitCommand([binding,  name, value]() mutable
			{
				ShaderUniformBufferObject buffer = OpenGLShader::s_UniformBuffers[binding];
				MemberUniformBufferObject uniform = buffer.Uniforms[name];
				RADIANT_VERIFY(uniform.Name != "");

				if (buffer.Name.empty())
					RA_WARN("[ OpenGLMaterial::SetUniform ] bufferName is empty");

				glBindBuffer(GL_UNIFORM_BUFFER, buffer.RenderingID);
				glBufferSubData(GL_UNIFORM_BUFFER, uniform.Offset, uniform.Size, &value[0]);
				glBindBuffer(GL_UNIFORM_BUFFER, 0);
			});
	}

	void OpenGLMaterial::SetUBO(BindingPoint binding, const std::string& name, const glm::mat4& value)
	{
		Rendering::SubmitCommand([binding,  name, value]() mutable
			{
				ShaderUniformBufferObject buffer = OpenGLShader::s_UniformBuffers[binding];
				MemberUniformBufferObject uniform = buffer.Uniforms[name];
				RADIANT_VERIFY(uniform.Name != "");

				if (buffer.Name.empty())
					RA_WARN("[ OpenGLMaterial::SetUniform ] bufferName is empty");

				glBindBuffer(GL_UNIFORM_BUFFER, buffer.RenderingID);
				glBufferSubData(GL_UNIFORM_BUFFER, uniform.Offset, uniform.Size, glm::value_ptr(value));
				glBindBuffer(GL_UNIFORM_BUFFER, 0);
			});
	}

	void OpenGLMaterial::SetUBO(BindingPoint binding, const std::string& name, float value)
	{
		Rendering::SubmitCommand([binding,  name, value]() mutable
			{
				ShaderUniformBufferObject buffer = OpenGLShader::s_UniformBuffers[binding];
				MemberUniformBufferObject uniform = buffer.Uniforms[name];
				RADIANT_VERIFY(uniform.Name != "");

				if (buffer.Name.empty())
					RA_WARN("[ OpenGLMaterial::SetUniform ] bufferName is empty");

				glBindBuffer(GL_UNIFORM_BUFFER, buffer.RenderingID);
				glBufferSubData(GL_UNIFORM_BUFFER, uniform.Offset, uniform.Size, &value);
				glBindBuffer(GL_UNIFORM_BUFFER, 0);
			});
	}

	void OpenGLMaterial::SetUBO(BindingPoint binding, const std::string& name, bool value)
	{
		Rendering::SubmitCommand([binding,  name, value]() mutable
			{
				ShaderUniformBufferObject buffer = OpenGLShader::s_UniformBuffers[binding];
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

	void OpenGLMaterial::SetUBO(BindingPoint binding, const std::string& name, const void* data, std::size_t size)
	{
		Memory::Buffer uboBuffer = Memory::Buffer::Copy(data, size);
		Rendering::SubmitCommand([binding, name, uboBuffer]() mutable
			{
				ShaderUniformBufferObject ubuffer = OpenGLShader::s_UniformBuffers[binding];
				MemberUniformBufferObject uniform = ubuffer.Uniforms[name];
				RADIANT_VERIFY(uniform.Name != "");
				RADIANT_VERIFY(uniform.Size == uboBuffer.Size);

				if (ubuffer.Name.empty())
					RA_WARN("[ OpenGLMaterial::SetUniform ] bufferName is empty");

				glBindBuffer(GL_UNIFORM_BUFFER, ubuffer.RenderingID);
				glBufferSubData(GL_UNIFORM_BUFFER, uniform.Offset, uboBuffer.Size, uboBuffer.Data);
				glBindBuffer(GL_UNIFORM_BUFFER, 0);

				uboBuffer.Release();
				// glNamedBufferSubData
			});
	}

	void OpenGLMaterial::UpdateForRendering() const
	{

	}

	void OpenGLMaterial::SetImage2D(const std::string& name, const Memory::Shared<Image2D>& image2D, uint32_t sampler) const
	{
		if (!image2D)
			return;
		Memory::Shared<const OpenGLMaterial> instance(this);
		Rendering::SubmitCommand([name, instance, image2D, sampler]() mutable
			{
				if (!image2D)
					return;

				const auto& samplerBuffer = instance->m_Shader.As<OpenGLShader>()->m_Resources[name];
				glBindTextureUnit(samplerBuffer.Uniform.Binding, image2D->GetTextureID());
				if (!sampler)
					sampler = image2D.As<OpenGLImage2D>()->GetSamplerID();
				glBindSampler(samplerBuffer.Uniform.Binding, sampler);
			});
	}

	void OpenGLMaterial::SetImage2D(const std::string& name, const Memory::Shared<Texture2D>& texture2D, uint32_t sampler) const
	{
		SetImage2D(name, texture2D->GetImage2D(), sampler);
	}

	void OpenGLMaterial::SetMat4(const std::string& name, const glm::mat4& value) const
	{
		if (m_Shader.As<OpenGLShader>()->m_Uniforms.find(name) == m_Shader.As<OpenGLShader>()->m_Uniforms.end())
			RADIANT_VERIFY(false);

		Memory::Shared<const OpenGLMaterial> instance(this);
		Rendering::SubmitCommand([name, instance, value]() mutable
			{
				const auto& buffer = instance->m_Shader.As<OpenGLShader>()->m_Uniforms[name];
				// RADIANT_VERIFY(buffer.Uniform.Binding != (uint32_t)-1);

				// instance->m_BufferValues.Write((void*)&value, buffer.Size, buffer.totalOffset);

				glUseProgram(instance->m_Shader.As<OpenGLShader>()->m_RenderingID);
				glUniformMatrix4fv(buffer.Uniform.Binding, 1, GL_FALSE, glm::value_ptr(value));
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
				// RADIANT_VERIFY(buffer.Uniform.Binding != (uint32_t)-1);

				// instance->m_BufferValues.Write((void*)&value, buffer.Size, buffer.totalOffset);

				glUseProgram(instance->m_Shader.As<OpenGLShader>()->m_RenderingID);
				glUniform1i(buffer.Uniform.Binding, value ? 1 : 0);
				glUseProgram(0);
			});
	}

	void OpenGLMaterial::SetUint(const std::string& name, uint32_t value) const
	{
		if (m_Shader.As<OpenGLShader>()->m_Uniforms.find(name) == m_Shader.As<OpenGLShader>()->m_Uniforms.end())
			RADIANT_VERIFY(false);

		Memory::Shared<const OpenGLMaterial> instance(this);
		Rendering::SubmitCommand([name, instance, value]() mutable
			{
				const auto& buffer = instance->m_Shader.As<OpenGLShader>()->m_Uniforms[name];
				// RADIANT_VERIFY(buffer.Uniform.Binding != (uint32_t)-1);

				// instance->m_BufferValues.Write((void*)&value, buffer.Size, buffer.totalOffset);

				glUseProgram(instance->m_Shader.As<OpenGLShader>()->m_RenderingID);
				glUniform1ui(buffer.Uniform.Binding, value);
				glUseProgram(0);
			});
	}

	void OpenGLMaterial::SetFloat(const std::string& name, float value) const
	{
		if (m_Shader.As<OpenGLShader>()->m_Uniforms.find(name) == m_Shader.As<OpenGLShader>()->m_Uniforms.end())
			RADIANT_VERIFY(false);

		Memory::Shared<const OpenGLMaterial> instance(this);
		Rendering::SubmitCommand([name, instance, value]() mutable
			{
				const auto& buffer = instance->m_Shader.As<OpenGLShader>()->m_Uniforms[name];
				// RADIANT_VERIFY(buffer.Uniform.Binding != (uint32_t)-1);

				// instance->m_BufferValues.Write((void*)&value, buffer.Size, buffer.totalOffset);

				glUseProgram(instance->m_Shader.As<OpenGLShader>()->m_RenderingID);
				glUniform1f(buffer.Uniform.Binding, value);
				glUseProgram(0);
			});
	}

	void OpenGLMaterial::SetVec3(const std::string& name, const glm::vec3 value) const
	{
		if (m_Shader.As<OpenGLShader>()->m_Uniforms.find(name) == m_Shader.As<OpenGLShader>()->m_Uniforms.end())
			RADIANT_VERIFY(false);

		Memory::Shared<const OpenGLMaterial> instance(this);
		Rendering::SubmitCommand([name, instance, value]() mutable
			{
				const auto& buffer = instance->m_Shader.As<OpenGLShader>()->m_Uniforms[name];
				// RADIANT_VERIFY(buffer.Uniform.Binding != (uint32_t)-1);

				// instance->m_BufferValues.Write((void*)&value, buffer.Size, buffer.totalOffset);

				glUseProgram(instance->m_Shader.As<OpenGLShader>()->m_RenderingID);
				glUniform3f(buffer.Uniform.Binding, value.x, value.y, value.z);
				glUseProgram(0);
			});
	}

	void OpenGLMaterial::SetVec4(const std::string& name, const glm::vec4 value) const
	{
		if (m_Shader.As<OpenGLShader>()->m_Uniforms.find(name) == m_Shader.As<OpenGLShader>()->m_Uniforms.end())
			RADIANT_VERIFY(false);

		Memory::Shared<const OpenGLMaterial> instance(this);
		Rendering::SubmitCommand([name, instance, value]() mutable
			{
				const auto& buffer = instance->m_Shader.As<OpenGLShader>()->m_Uniforms[name];
				// RADIANT_VERIFY(buffer.Uniform.Binding != (uint32_t)-1);

				// instance->m_BufferValues.Write((void*)&value, buffer.Size, buffer.totalOffset);

				glUseProgram(instance->m_Shader.As<OpenGLShader>()->m_RenderingID);
				glUniform4f(buffer.Uniform.Binding, value.x, value.y, value.z, value.w);
				glUseProgram(0);
			});
	}

}