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

	void OpenGLMaterial::UpdateForRendering() const
	{

	}

	void OpenGLMaterial::SetImage2D(const TextureDescriptor& descriptor, const Memory::Shared<Image2D>& image2D) const
	{
		if (!image2D)
			return;
		Memory::Shared<const OpenGLMaterial> instance(this);
		Rendering::SubmitCommand([descriptor, instance, image2D]() mutable
			{
				if (!image2D)
					return;

				const auto& samplerBuffer = instance->m_Shader.As<OpenGLShader>()->m_Resources[descriptor.Name];	
				if (descriptor.ArrayIndex.has_value())
				{
					RADIANT_VERIFY(samplerBuffer.Uniform.ArraySize > 1); //NOTE: texture must be arrayed
				}

				const auto binding = samplerBuffer.Uniform.Binding + descriptor.ArrayIndex.value_or(0);// NOTE: Using for array textures

				glBindTextureUnit(binding, image2D->GetTextureID());
				glBindSampler(binding, descriptor.Sampler.value_or(image2D.As<OpenGLImage2D>()->GetSamplerID()));
			});
	}

	void OpenGLMaterial::SetImage2D(const TextureDescriptor& descriptor, const Memory::Shared<Texture2D>& texture2D) const
	{
		if (!texture2D)
			return;
		SetImage2D(descriptor, texture2D->GetImage2D());
	}

	const Radiant::SamplerUniform& OpenGLMaterial::GetSamplerInformation(const std::string& name) const
	{
		const auto& buffer = m_Shader.As<OpenGLShader>()->m_Resources[name];

		return buffer;
	}

	void OpenGLMaterial::SetUniform(const std::string& name, RadiantShaderDataType type, const void* value, std::optional<uint32_t> arrayIndex) const
	{
		if (m_Shader.As<OpenGLShader>()->m_Uniforms.find(name) == m_Shader.As<OpenGLShader>()->m_Uniforms.end())
			RADIANT_VERIFY(false);

		Memory::Shared<const OpenGLMaterial> instance(this);
		Memory::Buffer bufferValue = Memory::Buffer::Copy(value, 255); //NOTE: maybe 255 is low? 
		Rendering::SubmitCommand([name, instance, type, bufferValue, arrayIndex]() mutable
			{
				const auto& buffer = instance->m_Shader.As<OpenGLShader>()->m_Uniforms[name];
				if (arrayIndex.has_value())
				{
					RADIANT_VERIFY(buffer.Uniform.ArraySize > 1); //NOTE: texture must be arrayed
				}

				// RADIANT_VERIFY(buffer.Uniform.Binding != (uint32_t)-1);

				// instance->m_BufferValues.Write((void*)&value, buffer.Size, buffer.totalOffset);

				const auto binding = buffer.Uniform.Binding + arrayIndex.value_or(0);

				glUseProgram(instance->m_Shader.As<OpenGLShader>()->m_RenderingID);
				switch (type)
				{
				case RadiantShaderDataType::Float:
					glUniform1f(binding, bufferValue.Read<float>());
					break;
				case RadiantShaderDataType::Int:
					glUniform1i(binding, bufferValue.Read<int>());
					break;
				case RadiantShaderDataType::Bool:
					glUniform1i(binding, bufferValue.Read<bool>() ? 1 : 0);
					break;
				case RadiantShaderDataType::UInt:
					glUniform1ui(binding, bufferValue.Read<uint32_t>());
					break;
				case RadiantShaderDataType::Float3:
				{
					glm::vec3 vecValue = bufferValue.Read<glm::vec3>();
					glUniform3f(binding, vecValue.x, vecValue.y, vecValue.z);
					break;
				}
				case RadiantShaderDataType::Float4:
				{
					glm::vec4 vecValue = bufferValue.Read<glm::vec4>();
					glUniform4f(binding, vecValue.x, vecValue.y, vecValue.z, vecValue.w);
					break;
				}
				case RadiantShaderDataType::Mat4:
				{
					glm::mat4 newValue = bufferValue.Read<glm::mat4>();
					glUniformMatrix4fv(binding, 1, GL_FALSE, glm::value_ptr(newValue));
					break;
				}
				default:
					RADIANT_VERIFY(false, "Unsupported type"); // Unsupported type
				}
				glUseProgram(0);
			});
	}

	void OpenGLMaterial::SetMat4(const std::string& name, const glm::mat4& value, std::optional<uint32_t> arrayIndex) const
	{
		SetUniform(name, RadiantShaderDataType::Mat4, &value, arrayIndex);
	}

	void OpenGLMaterial::SetBool(const std::string& name, bool value, std::optional<uint32_t> arrayIndex) const
	{
		SetUniform(name, RadiantShaderDataType::Bool, &value, arrayIndex);
	}

	void OpenGLMaterial::SetUint(const std::string& name, uint32_t value, std::optional<uint32_t> arrayIndex) const
	{
		SetUniform(name, RadiantShaderDataType::UInt, &value, arrayIndex);
	}

	void OpenGLMaterial::SetFloat(const std::string& name, float value, std::optional<uint32_t> arrayIndex) const
	{
		SetUniform(name, RadiantShaderDataType::Float, &value, arrayIndex);
	}

	void OpenGLMaterial::SetVec3(const std::string& name, const glm::vec3 value, std::optional<uint32_t> arrayIndex) const
	{
		SetUniform(name, RadiantShaderDataType::Float3, &value, arrayIndex);
	}

	void OpenGLMaterial::SetVec4(const std::string& name, const glm::vec4 value, std::optional<uint32_t> arrayIndex) const
	{
		SetUniform(name, RadiantShaderDataType::Float4, &value, arrayIndex);
	}

	void OpenGLMaterial::SetUBOMember(BindingPoint binding, const std::string& memberName, const glm::vec3& value)
	{
		Rendering::SubmitCommand([binding, memberName, value]() mutable
			{
				ShaderUniformBufferObject buffer = OpenGLShader::s_UniformBuffers[binding];
				MemberUniformBufferObject uniform = buffer.Uniforms[memberName];
				RADIANT_VERIFY(uniform.Name != "");

				if (buffer.Name.empty())
					RA_WARN("[ OpenGLMaterial::SetUniform ] bufferName is empty");

				glBindBuffer(GL_UNIFORM_BUFFER, buffer.RenderingID);
				glBufferSubData(GL_UNIFORM_BUFFER, uniform.Offset, uniform.Size, &value[0]);
				glBindBuffer(GL_UNIFORM_BUFFER, 0);
			});
	}

	void OpenGLMaterial::SetUBOMember(BindingPoint binding, const std::string& memberName, const glm::vec2& value)
	{
		Rendering::SubmitCommand([binding, memberName, value]() mutable
			{
				ShaderUniformBufferObject buffer = OpenGLShader::s_UniformBuffers[binding];
				MemberUniformBufferObject uniform = buffer.Uniforms[memberName];
				RADIANT_VERIFY(uniform.Name != "");

				if (buffer.Name.empty())
					RA_WARN("[ OpenGLMaterial::SetUniform ] bufferName is empty");

				glBindBuffer(GL_UNIFORM_BUFFER, buffer.RenderingID);
				glBufferSubData(GL_UNIFORM_BUFFER, uniform.Offset, uniform.Size, &value[0]);
				glBindBuffer(GL_UNIFORM_BUFFER, 0);
			});
	}

	void OpenGLMaterial::SetUBOMember(BindingPoint binding, const std::string& memberName, const glm::mat4& value)
	{
		Rendering::SubmitCommand([binding, memberName, value]() mutable
			{
				ShaderUniformBufferObject buffer = OpenGLShader::s_UniformBuffers[binding];
				MemberUniformBufferObject uniform = buffer.Uniforms[memberName];
				RADIANT_VERIFY(uniform.Name != "");

				if (buffer.Name.empty())
					RA_WARN("[ OpenGLMaterial::SetUniform ] bufferName is empty");

				glBindBuffer(GL_UNIFORM_BUFFER, buffer.RenderingID);
				glBufferSubData(GL_UNIFORM_BUFFER, uniform.Offset, uniform.Size, glm::value_ptr(value));
				glBindBuffer(GL_UNIFORM_BUFFER, 0);
			});
	}

	void OpenGLMaterial::SetUBOMember(BindingPoint binding, const std::string& memberName, float value)
	{
		Rendering::SubmitCommand([binding, memberName, value]() mutable
			{
				ShaderUniformBufferObject buffer = OpenGLShader::s_UniformBuffers[binding];
				MemberUniformBufferObject uniform = buffer.Uniforms[memberName];
				RADIANT_VERIFY(uniform.Name != "");

				if (buffer.Name.empty())
					RA_WARN("[ OpenGLMaterial::SetUniform ] bufferName is empty");

				glBindBuffer(GL_UNIFORM_BUFFER, buffer.RenderingID);
				glBufferSubData(GL_UNIFORM_BUFFER, uniform.Offset, uniform.Size, &value);
				glBindBuffer(GL_UNIFORM_BUFFER, 0);
			});
	}

	void OpenGLMaterial::SetUBOMember(BindingPoint binding, const std::string& memberName, bool value)
	{
		Rendering::SubmitCommand([binding, memberName, value]() mutable
			{
				ShaderUniformBufferObject buffer = OpenGLShader::s_UniformBuffers[binding];
				MemberUniformBufferObject uniform = buffer.Uniforms[memberName];
				RADIANT_VERIFY(uniform.Name != "");

				if (buffer.Name.empty())
					RA_WARN("[ OpenGLMaterial::SetUniform ] bufferName is empty");

				glBindBuffer(GL_UNIFORM_BUFFER, buffer.RenderingID);
				glBufferSubData(GL_UNIFORM_BUFFER, uniform.Offset, uniform.Size, &value);
				glBindBuffer(GL_UNIFORM_BUFFER, 0);

				// glNamedBufferSubData
			});
	}

	void OpenGLMaterial::SetUBO(BindingPoint binding, const void* data, std::size_t size, std::size_t offset)
	{
		Memory::Buffer storageBuffer = Memory::Buffer::Copy(data, size);
		Rendering::SubmitCommand([binding, offset, storageBuffer]() mutable
			{
				ShaderUniformBufferObject ubuffer = OpenGLShader::s_UniformBuffers[binding];
				RADIANT_VERIFY(ubuffer.Name != "");
				RADIANT_VERIFY(ubuffer.Size == storageBuffer.Size);

				if (ubuffer.Name.empty())
					RA_WARN("[ OpenGLMaterial::SetUniform ] bufferName is empty");

				glBindBuffer(GL_UNIFORM_BUFFER, ubuffer.RenderingID);
				glBufferSubData(GL_UNIFORM_BUFFER, offset, storageBuffer.Size, storageBuffer.Data);
				glBindBuffer(GL_UNIFORM_BUFFER, 0);

				storageBuffer.Release();
				// glNamedBufferSubData
			});
	}
}