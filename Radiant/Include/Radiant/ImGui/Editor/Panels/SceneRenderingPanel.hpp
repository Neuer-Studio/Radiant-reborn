#pragma once

#include <Radiant/Scene/Scene.hpp>

namespace Radiant
{
	class SceneRenderingPanel final : public Memory::RefCounted
	{
	public:
		SceneRenderingPanel(const Memory::Shared<Scene>& scene = nullptr);
		void SetContext(const Memory::Shared<Scene>& scene) { m_Context = scene; }

		void DrawImGuiUI();
	private:
		Memory::Shared<Scene> m_Context;
	};
}