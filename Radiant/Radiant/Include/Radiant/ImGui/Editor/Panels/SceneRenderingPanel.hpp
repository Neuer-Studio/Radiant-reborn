#pragma once

#include <Radiant/Scene/Scene.hpp>

namespace Radiant
{
    class SceneRenderingPanel final : public Common::Memory::RefCounted
    {
    public:
        SceneRenderingPanel( const Common::Memory::Shared<Scene>& scene = nullptr );
        void SetContext( const Common::Memory::Shared<Scene>& scene )
        {
            m_Context = scene;
        }

        void DrawImGuiUI();

    private:
        Common::Memory::Shared<Scene> m_Context;
    };
} // namespace Radiant