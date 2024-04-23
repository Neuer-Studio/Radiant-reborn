#pragma once

#include <Radiant/Rendering/Texture.hpp>
#include <Radiant/Scene/Scene.hpp>

namespace Radiant
{
	class Scene;

	class SceneRendering
	{
	public:
		~SceneRendering();

		static void BeginScene(Memory::Shared<Scene> m_Scene, const Camera& camera);
		static void EndScene();
		static void OnUpdate(Timestep ts);
		static void Init();

		static void SetSceneVeiwPortSize(const glm::vec2& size);
		static void SetEnvironment(const Environment& env);

		static void SetEnvironmentAttributes(const EnvironmentAttributes& attributes);

		static void SetEnvMapRotation(float rotation);
		static void SetIBLContribution(float value);
		static void OnImGuiRender();

		static void SubmitMesh(const Memory::Shared<Mesh>& mesh, const glm::mat4& transform);

		static Memory::Shared<Image2D> GetFinalPassImage();
		static Memory::Shared<Image2D> GetShadowMapPassImage();

		[[nodiscard]] static Environment CreateEnvironmentMap(const std::filesystem::path& filepath);
	};
}