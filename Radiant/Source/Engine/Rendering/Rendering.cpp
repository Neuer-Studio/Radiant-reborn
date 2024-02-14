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
		Memory::Shared<Pipeline> FullscreenQuadPipeline;
	};

	struct RenderingData
	{
		QuadData QuadInfo;
	};

	static RenderingData* s_RenderingData = nullptr;

	static Memory::Shared<RenderingContext> s_RenderingContext = nullptr;
	static Memory::Shared<RenderingAPI> s_RenderingAPIPlatform = nullptr;
	static Memory::CommandBuffer s_CommandBuffer;

	void Rendering::Clear(float rgba[4])
	{
		static float _rgba[4] = { rgba[0], rgba[1], rgba[2], rgba[3] };
		Rendering::SubmitCommand([]()
			{
				s_RenderingAPIPlatform->Clear((float*)_rgba);
			});
	}

	void Rendering::DrawPrimitive(Primitives primitive, uint32_t count, bool depthTest)
	{
		Rendering::SubmitCommand([primitive, count, depthTest]()
			{
				s_RenderingAPIPlatform->DrawPrimitive(primitive, count, depthTest);
			});
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

		PipelineSpecification pipelineSpecification;
		pipelineSpecification.Layout = {
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float2, "a_TexCoord" }
		};
		s_RenderingData->QuadInfo.FullscreenQuadPipeline = Pipeline::Create(pipelineSpecification);
		s_RenderingData->QuadInfo.FullscreenQuadVertexBuffer = VertexBuffer::Create(data, 4 * sizeof(QuadVertex));
		uint32_t indices[6] = { 0, 1, 2, 2, 3, 0, };
		s_RenderingData->QuadInfo.FullscreenQuadIndexBuffer = IndexBuffer::Create(indices, 6 * sizeof(uint32_t));

		return s_RenderingContext;
	}

	Radiant::Memory::Shared<Radiant::RenderingContext> Rendering::GetRenderingContext()
	{
		return s_RenderingContext;
	}

	void Rendering::DrawFullscreenQuad()
	{
		s_RenderingData->QuadInfo.FullscreenQuadVertexBuffer->Use();
		s_RenderingData->QuadInfo.FullscreenQuadPipeline->Use();
		s_RenderingData->QuadInfo.FullscreenQuadIndexBuffer->Use();

		DrawPrimitive(Primitives::Triangle, s_RenderingData->QuadInfo.FullscreenQuadIndexBuffer->GetCount(), false);
	}

	Memory::CommandBuffer& Rendering::GetRenderingCommandBuffer()
	{
		return s_CommandBuffer;
	}
}
