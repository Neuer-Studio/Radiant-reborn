#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <Radiant/Rendering/Scene/Entity.hpp>
#include <Radiant/Rendering/Scene/SceneRendering.hpp>

namespace Radiant
{
	class EditorLayer : public Layer
	{
	public:
		EditorLayer()
			: Layer("EditorLayer"), CAM(glm::perspectiveFov(glm::radians(45.0f), 1280.0f, 720.0f, 0.1f, 10000.0f))
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
			sc = Shader::Create("Resources/Shaders/Scene.glsl");
			tex = Texture2D::Create("Resources/Textures/HDR/birchwood_4k.hdr");
			VertexBufferLayout vertexLayout;
			vertexLayout = {
					{ ShaderDataType::Float3, "a_Position" },
					{ ShaderDataType::Float3, "a_Normals" },
					//{ ShaderDataType::Float2, "a_TexCoords" },
			};
			PipelineSpecification pipelineSpecification;
			pipelineSpecification.Layout = vertexLayout;
			pip = Pipeline::Create(pipelineSpecification);

			MAT = Material::Create(sh);
			MAT2 = Material::Create(sc);

			MAT->SetUniform("Props", "color", glm::vec3(0.3f, 0.4f, 0.1f));
			//MAT->SetUniform("diffuseTexture", tex);

			MESH = new Mesh("Resources/Meshes/Cube1m.fbx");
			static auto sr = SceneRendering::Create("");
			auto ss = Scene::CreateScene(sr);
			MAT2->SetUniform("envTexture",ss.Radiance);

			fb = Framebuffer::Create({ 1920, 1080,1, ImageFormat::RGBA16F });
		}
		virtual void OnDetach()
		{

		}
		virtual void OnUpdate() override
		{
			CAM.OnUpdate();

			auto viewProjection = CAM.GetProjectionMatrix() * CAM.GetViewMatrix();
			MAT->SetUniform("Camera", "u_ViewProjectionMatrix", viewProjection);
			MAT2->SetUniform("TransformUniforms", "viewProjectionMatrix", viewProjection);

			pip->Use();
			MESH->Use();
		//	sh->Use();
			sc->Use();
			//tex->Use(1);
			Rendering::DrawPrimitive(Primitives::Triangle, MESH->GetIndexCount());

			CAM.SetProjectionMatrix(glm::perspectiveFov(glm::radians(45.0f), 1280.0f, 720.0f, 0.1f, 10000.0f));
			CAM.SetViewportSize((uint32_t)1280.0f, (uint32_t)720.0f);

		}
	private:
		Memory::Shared<VertexBuffer> VBO;
		Memory::Shared<Shader> sh;
		Memory::Shared<Shader> sc;
		Memory::Shared<Texture2D> tex;
		Memory::Shared<Pipeline> pip;
		Memory::Shared<IndexBuffer> IBO;
		Memory::Shared<Material> MAT;
		Memory::Shared<Material> MAT2;
		Memory::Shared<Mesh> MESH;
		Memory::Shared<Framebuffer> fb;
		Camera CAM;

	};
}