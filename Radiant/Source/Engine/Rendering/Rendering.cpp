#include <glad/glad.h>

#include <Radiant/Rendering/VertexBuffer.hpp>
#include <Radiant/Rendering/IndexBuffer.hpp>
#include <Radiant/Rendering/Pipeline.hpp>
#include <Radiant/Rendering/Rendering.hpp>
#include <Radiant/Rendering/Shader.hpp>
#include <Radiant/Rendering/Platform/OpenGL/OpenGLRenderingAPI.hpp>

namespace Radiant
{
	// =================================================================== //
	// ========================= Rendering API =========================== //

	static RenderingAPIType s_RenderingAPI = RenderingAPIType::None;

	const RenderingAPIType RenderingAPI::GetAPI()
	{
		return s_RenderingAPI;
	}

	void RenderingAPI::SetAPI(RenderingAPIType api)
	{
		s_RenderingAPI = api;
	}

	// ========================= Rendering API =========================== //
	// =================================================================== //

	struct QuadData
	{
		Memory::Shared<VertexBuffer> FullscreenQuadVertexBuffer;
		Memory::Shared<IndexBuffer> FullscreenQuadIndexBuffer;
	};

	struct RenderingData
	{
		QuadData QuadInfo;
		Memory::Shared<RenderPass> ActiveRenderPass;

		ShaderLibrary* s_ShaderLibrary = nullptr;
	};

	static RenderingData* s_RenderingData = nullptr;

	static Memory::Shared<RenderingContext> s_RenderingContext = nullptr;
	static Memory::Shared<RenderingAPI> s_RenderingAPIPlatform = nullptr;
	static Memory::CommandBuffer s_CommandBuffer;

	Rendering::~Rendering()
	{
		delete s_RenderingData->s_ShaderLibrary;
		delete s_RenderingData;

		s_CommandBuffer.Execute();
	}

	void Rendering::Clear(float rgba[4])
	{
		static float _rgba[4] = { rgba[0], rgba[1], rgba[2], rgba[3] };
		Rendering::SubmitCommand([]()
			{
				s_RenderingAPIPlatform->Clear((float*)_rgba);
			});
	}

	void Rendering::SubmitMesh(const Memory::Shared<Mesh>& mesh, const Memory::Shared<Pipeline>& pipeline)
	{
		mesh->GetVertexBuffer()->Use();
		pipeline->Use();
		mesh->GetIndexBuffer()->Use();

		Rendering::DrawPrimitive(Primitives::Triangle, mesh->GetIndexCount(), true);
	}

	void Rendering::DrawPrimitive(Primitives primitive, uint32_t count, bool depthTest)
	{
		Rendering::SubmitCommand([primitive, count, depthTest]()
			{
				s_RenderingAPIPlatform->DrawPrimitive(primitive, count, depthTest);
			});
	}

	void Rendering::BeginRenderPass(Memory::Shared <RenderPass>& renderPass, bool clear /*= true*/)
	{
		RADIANT_VERIFY(renderPass, "Render pass cannot be null!");

		// TODO: Convert all of this into a render command buffer
		s_RenderingData->ActiveRenderPass = renderPass;

		renderPass->GetSpecification().TargetFramebuffer->Use();
		if (clear)
		{
			static float rgba[4] = { 0.4f, 0.3f, 0.1f, 1.0f }; //TODO: Get from Specification
			Rendering::Clear(rgba);
		}

	}

	void Rendering::EndRenderPass()
	{
		RADIANT_VERIFY(s_RenderingData->ActiveRenderPass, "No active render pass! Have you called Renderer::EndRenderPass twice?");
		s_RenderingData->ActiveRenderPass->GetSpecification().TargetFramebuffer->Use(BindUsage::Unbind);
		s_RenderingData->ActiveRenderPass = nullptr;

	}

	Memory::Shared<RenderingContext> Rendering::Initialize(GLFWwindow* window)
	{
		switch (RenderingAPI::GetAPI())
		{
			case RenderingAPIType::OpenGL:
			{
				s_RenderingAPIPlatform = Memory::Shared<OpenGLRenderingAPI>::Create();
			}
		}
		RADIANT_VERIFY(s_RenderingAPIPlatform);
		s_RenderingContext = RenderingContext::Create(window);

		s_RenderingData = new RenderingData();
		s_RenderingData->s_ShaderLibrary = new ShaderLibrary();

		// NOTE(Danya): Create fullscreen quad
		float x = -1;
		float y = -1;
		float width = 2, height = 2;
		struct QuadVertex
		{
			glm::vec3 Position;
			glm::vec2 TexCoord;
		};

		QuadVertex* data = new QuadVertex[4];

		data[0].Position = glm::vec3(x, y, 0.1f);
		data[0].TexCoord = glm::vec2(0, 0);

		data[1].Position = glm::vec3(x + width, y, 0.1f);
		data[1].TexCoord = glm::vec2(1, 0);

		data[2].Position = glm::vec3(x + width, y + height, 0.1f);
		data[2].TexCoord = glm::vec2(1, 1);

		data[3].Position = glm::vec3(x, y + height, 0.1f);
		data[3].TexCoord = glm::vec2(0, 1);

		s_RenderingData->QuadInfo.FullscreenQuadVertexBuffer = VertexBuffer::Create(data, 4 * sizeof(QuadVertex));
		uint32_t indices[6] = { 0, 1, 2, 2, 3, 0, };
		s_RenderingData->QuadInfo.FullscreenQuadIndexBuffer = IndexBuffer::Create(indices, 6 * sizeof(uint32_t));

		// Load Shaders

		{
			// Compute shaders

			s_RenderingData->s_ShaderLibrary->Load("Resources/Shaders/equirect2cube_cs.glsl");

			s_RenderingData->s_ShaderLibrary->Load("Resources/Shaders/Skybox.glsl");
			s_RenderingData->s_ShaderLibrary->Load("Resources/Shaders/StaticPBR_Radiant.glsl");
			s_RenderingData->s_ShaderLibrary->Load("Resources/Shaders/SceneComposite.glsl");
			s_RenderingData->s_ShaderLibrary->Load("Resources/Shaders/Grid.glsl");
		}

		return s_RenderingContext;
	}

	Radiant::Memory::Shared<Radiant::RenderingContext> Rendering::GetRenderingContext()
	{
		return s_RenderingContext;
	}

	const Radiant::ShaderLibrary* Rendering::GetShaderLibrary()
	{
		return s_RenderingData->s_ShaderLibrary;
	}

	void Rendering::SubmitFullscreenQuad(const Memory::Shared<Pipeline>& pipeline, const Memory::Shared<Material>& material)
	{
		if (!pipeline)
			return;
		bool depthTest = false;
		bool cullFace = false;
		if (material)
		{
			material->Use();
			depthTest = material->GetFlag(MaterialFlag::DepthTest); 
		}

		s_RenderingData->QuadInfo.FullscreenQuadVertexBuffer->Use();
		pipeline->Use();
		s_RenderingData->QuadInfo.FullscreenQuadIndexBuffer->Use();

		Rendering::SubmitCommand([material, depthTest]()
			{
				if (depthTest)
					glEnable(GL_DEPTH_TEST);
				else
					glDisable(GL_DEPTH_TEST);

				glDrawElements(GL_TRIANGLES, s_RenderingData->QuadInfo.FullscreenQuadIndexBuffer->GetCount(), GL_UNSIGNED_INT, nullptr);
			});
	}

	Memory::CommandBuffer& Rendering::GetRenderingCommandBuffer()
	{
		return s_CommandBuffer;
	}
}
