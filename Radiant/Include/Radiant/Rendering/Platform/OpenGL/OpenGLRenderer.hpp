#pragma once

#include <Radiant/Rendering/RendererAPI.hpp>

namespace Radiant
{
	class OpenGLRenderer : public RendererAPI
	{
	public:
		OpenGLRenderer() = default;
		virtual void Clear(float rgba[4]) const override;
		virtual void DrawPrimitive(Primitives primitive = Primitives::Triangle, uint32_t count = 3, bool depthTest = true) const override;
		virtual void SetLineWidth(float width) const override;

		virtual const Environment CreateEnvironmentMap(const std::filesystem::path& filepath) const override;
	};
}