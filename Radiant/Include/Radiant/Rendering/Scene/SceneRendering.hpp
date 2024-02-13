#pragma once

#include <Radiant/Rendering/Texture.hpp>
#include <Radiant/Rendering/Scene/Scene.hpp>

namespace Radiant
{
	struct Environment
	{
		Memory::Shared<Image2D> Radiance;
		Memory::Shared<Image2D> Irradiance;
	};

	class Scene;

	class SceneRendering : public Memory::RefCounted
	{
	public:
		virtual ~SceneRendering() = default;

		virtual void SubmitScene(Camera* cam) = 0;
		virtual void Init() = 0;

		virtual void SetSceneVeiwPortSize(const glm::vec2& size) = 0;
		virtual void SetEnvironment(const Environment& env) = 0;

		virtual Memory::Shared<Image2D> GetFinalPassImage() const = 0;

		virtual Environment CreateEnvironmentScene(const std::filesystem::path& filepath) const = 0;

		static Memory::Shared<SceneRendering> Create(const Memory::Shared<Scene>& scene);
				
	};
}