#pragma once

#include <Radiant/Rendering/Pipeline.hpp>

namespace Radiant
{
	class OpenGLPipeline final : public Pipeline
	{
	public:
		OpenGLPipeline(const PipelineSpecification& spec);
		virtual ~OpenGLPipeline();

		virtual PipelineSpecification& GetSpecification() { return m_Specification; }
		virtual const PipelineSpecification& GetSpecification() const { return m_Specification; }

		virtual void Invalidate() override;

		virtual void Use(BindUsage use = BindUsage::Bind, const std::vector<std::string_view>& attributesToEnable = {}) const override;
	private:
		PipelineSpecification m_Specification;
		RenderingID m_RendererID = 0;
	};
}