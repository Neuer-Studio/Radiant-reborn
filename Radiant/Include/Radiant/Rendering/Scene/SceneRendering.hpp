#pragma once

#include <Radiant/Rendering/Texture.hpp>

namespace Radiant
{
	struct Environment
	{
		Memory::Shared<Image2D> Radiance;
		Memory::Shared<Image2D> Irradiance;
	};

	class SceneRendering : public Memory::RefCounted
	{
	public:
		virtual ~SceneRendering() = default;

		virtual void SubmitScene() const = 0;
		virtual void Init() = 0;

		virtual Environment CreateEnvironmentScene(const std::filesystem::path& filepath) const = 0;

		static Memory::Shared<SceneRendering> Create(const std::string& sceneName);
				
	};
}