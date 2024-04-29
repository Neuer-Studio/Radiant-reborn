#pragma once

#include <Radiant/Rendering/Texture.hpp>
#include <Radiant/Scene/Scene.hpp>

namespace Radiant
{
	class Scene;

	class SceneRendering
	{
	public:
		static SceneRendering& Get();

		~SceneRendering();
	private:
		void BeginScene(Memory::Shared<Scene> m_Scene, const Camera& camera);
		void Init();
		void EndScene();
		void OnUpdate(Timestep ts);

		void SetSceneVeiwPortSize(const glm::vec2& size);
		void SetEnvironment(const Environment& env);

		void SetEnvironmentAttributes(const EnvironmentAttributes& attributes);

		void SetEnvMapRotation(float rotation);
		void SetIBLContribution(float value);
		void OnImGuiRender();

		void SubmitMesh(const Memory::Shared<Mesh>& mesh, const glm::mat4& transform);

		[[nodiscard]] static Memory::Shared<Image2D> GetFinalPassImage();
		static Memory::Shared<Image2D> GetShadowMapPassImage();

		[[nodiscard]] static Environment CreateEnvironmentMap(const std::filesystem::path& filepath);
	private:
		void FlushDrawList();
		void ShadowMapPass();
		void GeometryPass();
		void CompositePass();
	private:
		friend class Scene;
		friend class Rendering;
		friend class PanelOutliner;
		friend class Environment;
	};
}