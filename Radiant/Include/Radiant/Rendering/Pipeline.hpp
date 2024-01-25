#pragma once

#include "VertexBuffer.hpp"

namespace Radiant
{
	struct PipelineSpecification
	{
		VertexBufferLayout Layout;
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