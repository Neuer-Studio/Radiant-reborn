#include <Radiant/Rendering/2D/Rendering2D.hpp>
#include <Radiant/Rendering/Rendering.hpp>

namespace Radiant
{
	struct LinesCapacity
	{
		static constexpr uint32_t MaxLinesCount = 1000;
		static constexpr uint32_t LinesVertexCount = 1000 * 2;
		static constexpr uint32_t LinesIndexCount = 1000 * 6;
	};

	static bool s_BeginSceneCalled = false;

	Rendering2D& Rendering2D::Get()
	{
		static Rendering2D instance;
		return instance;
	}

	Rendering2D::~Rendering2D()
	{

	}

	void Rendering2D::Init()
	{
		// Lines 
		{
			const uint32_t lineVertexCount = LinesCapacity::LinesVertexCount;
			const uint32_t lineIndexCount = LinesCapacity::LinesIndexCount;
			m_RenderingData.LinesData.LineVertexBuffer = VertexBuffer::Create(lineVertexCount * sizeof(Lines::LineVertex));

			std::vector<uint32_t> indexBuffer(lineIndexCount);
			for (int i = 0; i < lineIndexCount; i++)
			{
				indexBuffer[i] = i;
			}


			m_RenderingData.LinesData.LineIndexBuffer = IndexBuffer::Create(indexBuffer.data(), lineIndexCount * sizeof(uint32_t));

			PipelineSpecification linesPiepline;
			linesPiepline.DebugName = "Lines-Pipeline";
			linesPiepline.Shader = Rendering::GetShaderLibrary()->Get("Rendering2D.glsl");
			linesPiepline.Layout = {
				{ ShaderDataType::Float3, "a_Position" }
			};

			m_RenderingData.LinesData.LinePipeline = Pipeline::Create(linesPiepline);

			m_RenderingData.LinesData.LineVertexData.resize(LinesCapacity::MaxLinesCount);
		}
	}

	void Rendering2D::Flush()
	{
		m_RenderingData.LinesData.LinesCount = 0;
	}

	void Rendering2D::BeginScene(const Scene2DInformation& information)
	{
		RADIANT_VERIFY(!s_BeginSceneCalled, "Have you call EndScene() ?");
		s_BeginSceneCalled = true;

		m_RenderingData.SceneInformation = information;
	}

	void Rendering2D::EndScene()
	{
		RADIANT_VERIFY(s_BeginSceneCalled, "Have you call BeginScene(information) ?");
		s_BeginSceneCalled = false;

		const float linesCount = m_RenderingData.LinesData.LinesCount;
		if (linesCount)
		{
			const auto lineWidth = m_RenderingData.LinesData.LineWidth;

			m_RenderingData.LinesData.LineVertexBuffer->Use();
			m_RenderingData.LinesData.LinePipeline->Use();
			m_RenderingData.LinesData.LineIndexBuffer->Use();
			m_RenderingData.LinesData.GetShader()->Use();

			Rendering::SetLineWidth(lineWidth);
			Rendering::DrawPrimitive(Primitives::Line, linesCount, false);
		}

		Flush();
	}

	void Rendering2D::DrawLine(const glm::vec3& p1, const glm::vec3& p2, float lineWidth)
	{
		RADIANT_VERIFY(s_BeginSceneCalled, "Have you call BeginScene(information) ?");

		auto& lineCount = m_RenderingData.LinesData.LinesCount;
		m_RenderingData.LinesData.LineWidth = lineWidth;

		m_RenderingData.LinesData.LineVertexData[(lineCount++)].Position = p1;
		m_RenderingData.LinesData.LineVertexData[(lineCount++)].Position = p2;
		m_RenderingData.LinesData.LineVertexBuffer->SetData(m_RenderingData.LinesData.LineVertexData.data(), lineCount * sizeof(Lines::LineVertex)); //TODO: should we use offset ? 
	}
}