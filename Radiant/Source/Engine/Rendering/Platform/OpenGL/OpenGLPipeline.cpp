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
		Rendering::Submit([rendererID]()
			{
				glDeleteVertexArrays(1, &rendererID);
			});
	}

	void OpenGLPipeline::Invalidate()
	{
		RADIANT_VERIFY(m_Specification.Layout.GetElements().size(), "Layout is empty!");

		Memory::Shared<OpenGLPipeline> instance = this;
		Rendering::Submit([instance]() mutable
			{
				auto& vertexArrayRendererID = instance->m_RendererID;

				if (vertexArrayRendererID)
					glDeleteVertexArrays(1, &vertexArrayRendererID);

				glGenVertexArrays(1, &vertexArrayRendererID);
				glBindVertexArray(vertexArrayRendererID);

				glBindVertexArray(0);
			});
	}

	void OpenGLPipeline::Bind() const
	{
		const Memory::Shared<const OpenGLPipeline> instance = this;
		Rendering::Submit([instance]()
			{
				glBindVertexArray(instance->m_RendererID);

				const auto& layout = instance->m_Specification.Layout;
				uint32_t attribIndex = 0;
				for (const auto& element : layout)
				{
					auto glBaseType = OpenGLShaderDataType(element.Type);
					glEnableVertexAttribArray(attribIndex);
					if (glBaseType == GL_INT)
					{
						glVertexAttribIPointer(attribIndex,
							element.GetComponentCount(),
							glBaseType,
							layout.GetStride(),
							(const void*)(intptr_t)element.Offset);
					}
					else
					{
						glVertexAttribPointer(attribIndex,
							element.GetComponentCount(),
							glBaseType,
							element.Normalized ? GL_TRUE : GL_FALSE,
							layout.GetStride(),
							(const void*)(intptr_t)element.Offset);
					}
					attribIndex++;


				}
			});
	}

	void OpenGLPipeline::Unbind() const
	{
		Rendering::Submit([]()
			{
				glBindVertexArray(0);
			});
	}

}