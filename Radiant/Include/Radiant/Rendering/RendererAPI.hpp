#pragma once

#include <Radiant/Rendering/RendererAPI.hpp>
#include <Radiant/Rendering/Environment.hpp>
#include <GLFW/glfw3.h>

namespace Radiant
{
	enum class RenderingAPIType : uint8_t
	{
		None = 0,
		Vulkan = 1,
		OpenGL = 2
	};

	enum class Primitives
	{
		Triangle = 0,
		Line
	};

	struct GraphicsInfo {
		std::string Vendor;
		std::string Renderer;
		std::string Version;

		int MaxSamples = 0;
		float MaxAnisotropy = 0.0f; // Texture filtering
		int MaxTextureUnits = 0;
	};

	class RendererAPI : public Memory::RefCounted
	{
	public:
		static GraphicsInfo& GetGraphicsInfo()
		{
			static GraphicsInfo info;
			return info;
		}

		virtual void Clear(float rgba[4]) const = 0;
		virtual void DrawPrimitive(Primitives primitive = Primitives::Triangle, uint32_t count = 3, bool depthTest = true) const = 0;
	public:
		virtual const Environment CreateEnvironmentMap(const std::filesystem::path& filepath) const = 0;
	public:
		static const RenderingAPIType GetAPI();
		static void SetAPI(RenderingAPIType api);
	};
}