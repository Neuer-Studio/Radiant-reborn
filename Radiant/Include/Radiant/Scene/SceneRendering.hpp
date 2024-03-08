#pragma once

#include <Radiant/Rendering/Texture.hpp>
#include <Radiant/Scene/Scene.hpp>

namespace Radiant
{
	struct Environment
	{
		Memory::Shared<Image2D> Radiance;
		Memory::Shared<Image2D> Irradiance;
	};

	class Scene;

	class SceneRendering
	{
	public:
		virtual ~SceneRendering() = default;

		virtual void BeginScene(const Camera& camera) = 0;
		virtual void OnUpdate(Timestep ts) = 0;
		virtual void Init() = 0;

		virtual void SetSceneVeiwPortSize(const glm::vec2& size) = 0;
		virtual void SetEnvironment(const Environment& env) = 0;

		virtual void SetEnvMapRotation(float rotation) = 0;
		virtual void SetRadiancePrefilter(bool enable = true) = 0;

		virtual void SubmitMesh(const Memory::Shared<Mesh>& mesh, const glm::mat4& transform) const = 0;

		virtual Memory::Shared<Image2D> GetFinalPassImage() const = 0;

		virtual Environment CreateEnvironmentScene(const std::filesystem::path& filepath) const = 0;

		static SceneRendering* Create(const Memory::Shared<Scene>& scene);
				
	};
}