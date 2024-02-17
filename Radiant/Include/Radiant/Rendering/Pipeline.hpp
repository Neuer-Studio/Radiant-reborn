#pragma once

#include "VertexBuffer.hpp"
#include <Radiant/Rendering/RenderPass.hpp>
#include <Radiant/Rendering/Shader.hpp>
#include <Radiant/Rendering/RenderPass.hpp>

namespace Radiant
{
	struct PipelineSpecification
	{
		Memory::Shared<Shader> Shader;
		Memory::Shared<RenderPass> RenderPass;
		VertexBufferLayout Layout;

		std::string DebugName;
	};

	class Pipeline : public Memory::RefCounted
	{
	public:
		virtual ~Pipeline() = default;

		virtual PipelineSpecification& GetSpecification() = 0;
		virtual const PipelineSpecification& GetSpecification() const = 0;

		virtual void Invalidate() = 0;

		virtual void Use(BindUsage use = BindUsage::Bind) const = 0;

		static Memory::Shared<Pipeline> Create(const PipelineSpecification& spec);
	};
}