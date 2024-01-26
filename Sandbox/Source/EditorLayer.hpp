#pragma once

namespace Radiant
{
	class EditorLayer : public Layer
	{
	public:
		EditorLayer()
			: Layer("EditorLayer")
		{}

		virtual void OnAttach()
		{
			static float v []=
			{
				// positions           
				0.5f, 0.5f, 0.0f,				1.0f, 1.0f, // top right
				0.5f, -0.5f, 0.0f,				 1.0f, 0.0f, // bottom right
				-0.5f, -0.5f, 0.0f,					 0.0f, 0.0f, // bottom left
				-0.5f, 0.5f, 0.0f,					0.0f, 1.0f  // top left 
			};
			static unsigned int indices[] = {
				0, 1, 3, // first triangle
				1, 2, 3  // second triangle
			};
			IBO = IndexBuffer::Create(indices, sizeof(indices));
			VBO = VertexBuffer::Create((std::byte*)v, sizeof(v));
			sh = Shader::Create("Resources/Shaders/test.radiantshader");
			tex = Texture2D::Create("Resources/Textures/awesomeface.png");
			VertexBufferLayout vertexLayout;
			vertexLayout = {
					{ ShaderDataType::Float3, "a_Position" },
					{ ShaderDataType::Float2, "a_TexCoords" },
			};
			PipelineSpecification pipelineSpecification;
			pipelineSpecification.Layout = vertexLayout;
			pip = Pipeline::Create(pipelineSpecification);

			MAT = Material::Create(sh);

			MAT->SetUniform(RadiantShaderType::Fragment, 0, "color", glm::vec3(0.3f, 0.4f, 0.1f));
			MAT->SetUniform("diffuseTexture", tex);
		}
		virtual void OnDetach()
		{

		}
		virtual void OnUpdate() override
		{

			pip->Use();
			VBO->Use();
			IBO->Use();
			sh->Use();
			//tex->Use(1);
			
			Rendering::DrawPrimitive(Primitives::Triangle, IBO->GetCount());
		}
	private:
		Memory::Shared<VertexBuffer> VBO;
		Memory::Shared<Shader> sh;
		Memory::Shared<Texture2D> tex;
		Memory::Shared<Pipeline> pip;
		Memory::Shared<IndexBuffer> IBO;
		Memory::Shared<Material> MAT;


	};
}