#pragma once

#include <glm/glm.hpp>

#include <Radiant/Rendering/Pipeline.hpp>
#include <Radiant/Rendering/VertexBuffer.hpp>
#include <Radiant/Rendering/IndexBuffer.hpp>
#include <Radiant/Rendering/Mesh.hpp>

namespace Radiant
{
	struct Scene2DInformation
	{
		bool DepthTest;
	};

	class Rendering2D
	{
	public:
		static Rendering2D& Get();

		~Rendering2D();
	private:
		void Init();
		void Flush();

		void BeginScene(const Scene2DInformation& information);
		void EndScene();

		void DrawLine(const glm::vec3& p1, const glm::vec3& p2, float lineWidth = 1.0f);
	private:
		struct Lines
		{
			struct LineVertex
			{
				glm::vec3 Position;
			};

			Memory::Shared<VertexBuffer> LineVertexBuffer;
			Memory::Shared<IndexBuffer> LineIndexBuffer;
			Memory::Shared<Pipeline> LinePipeline;
			std::vector<LineVertex> LineVertexData;
			uint32_t LinesCount;
			float LineWidth;

			const Memory::Shared<Shader>& GetShader() const { return LinePipeline->GetSpecification().Shader; }
		};

		struct RenderingData
		{
			Lines LinesData;

			Scene2DInformation SceneInformation;
		} m_RenderingData;
	private:
		friend class Rendering;
		friend class SceneRendering;
	};
}