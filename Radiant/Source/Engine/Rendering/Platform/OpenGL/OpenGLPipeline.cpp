#include <glad/glad.h>

#include <Radiant/Rendering/Platform/OpenGL/OpenGLPipeline.hpp>
#include <Radiant/Rendering/Rendering.hpp>

namespace Radiant
{
	static GLenum OpenGLShaderDataType(ShaderDataType type)
	{
		switch (type)
		{
		case ShaderDataType::Float:    return GL_FLOAT;
		case ShaderDataType::Float2:   return GL_FLOAT;
		case ShaderDataType::Float3:   return GL_FLOAT;
		case ShaderDataType::Float4:   return GL_FLOAT;
		case ShaderDataType::Int:      return GL_INT;
		case ShaderDataType::Int2:     return GL_INT;
		case ShaderDataType::Int3:     return GL_INT;
		case ShaderDataType::Int4:     return GL_INT;
		case ShaderDataType::Bool:     return GL_BOOL;
		}

		RADIANT_VERIFY(false, "Unknown ShaderDataType!");
		return 0;
	}


	OpenGLPipeline::OpenGLPipeline(const PipelineSpecification& spec)
		: m_Specification(spec)
	{
		Invalidate();
	}

	OpenGLPipeline::~OpenGLPipeline()
	{
		GLuint rendererID = m_RendererID;
		Rendering::SubmitCommand([rendererID]()
			{
				glDeleteVertexArrays(1, &rendererID);
			});
	}

	void OpenGLPipeline::Invalidate()
	{
		RADIANT_VERIFY(m_Specification.Layout.GetElements().size(), "Layout is empty!");

		Memory::Shared<OpenGLPipeline> instance = this;
		Rendering::SubmitCommand([instance]() mutable
			{
				auto& vertexArrayRendererID = instance->m_RendererID;

				if (vertexArrayRendererID)
					glDeleteVertexArrays(1, &vertexArrayRendererID);

				glGenVertexArrays(1, &vertexArrayRendererID);
				glBindVertexArray(vertexArrayRendererID);

				glBindVertexArray(0);
			});
	}

	void OpenGLPipeline::Use(BindUsage use, const std::vector<std::string_view>& attributesToEnable) const
	{
		const Memory::Shared<const OpenGLPipeline> instance = this;
		Rendering::SubmitCommand([instance, use, attributesToEnable]() mutable
			{
				if (use == BindUsage::Unbind)
				{
					glBindVertexArray(0);
					return;
				}

				glBindVertexArray(instance->m_RendererID);

				const auto& layout = instance->m_Specification.Layout;
				uint32_t attribIndex = 0;

				auto setVertexAttr = [](ShaderDataType type, uint32_t attribIndex, uint32_t count, uint32_t stride, uint32_t offset, bool normalized) -> void
				{
					auto glBaseType = OpenGLShaderDataType(type);
					glEnableVertexAttribArray(attribIndex);
					if (glBaseType == GL_INT)
					{
						glVertexAttribIPointer(attribIndex,
							count,
							glBaseType,
							stride,
							(const void*)(intptr_t)offset);
					}
					else
					{
						glVertexAttribPointer(attribIndex,
							count,
							glBaseType,
							normalized ? GL_TRUE : GL_FALSE,
							stride,
							(const void*)(intptr_t)offset);
					}
				};

				if (attributesToEnable.empty())
				{
					for (const auto& element : layout)
					{
						setVertexAttr(element.Type, attribIndex, element.GetComponentCount(), layout.GetStride(), element.Offset, element.Normalized);
						attribIndex++;
					}
				}

				else
				{
					for (const auto& element : layout)
					{
						if (std::find(attributesToEnable.begin(), attributesToEnable.end(), element.Name) == std::end(attributesToEnable))
						{
							attribIndex++;
							continue;
						}

						setVertexAttr(element.Type, attribIndex, element.GetComponentCount(), layout.GetStride(), element.Offset, element.Normalized);
						attribIndex++;
					}
				}

			});
	}
}