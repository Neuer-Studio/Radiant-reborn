#pragma once

#include <Radiant/Rendering/RenderingAPI.hpp>

namespace Radiant
{
	class OpenGLRenderingAPI : public RenderingAPI
	{
	public:
		OpenGLRenderingAPI() = default;
		virtual void Clear(float rgba[4]) const override;
		virtual void DrawPrimitive(Primitives primitive = Primitives::Triangle, uint32_t count = 3) const override;
	};
}