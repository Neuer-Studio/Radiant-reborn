#pragma once

#include <Radiant/Rendering/Image.hpp>

namespace Radiant
{
    struct Environment
    {
        std::string             FilePath;
        Memory::Shared<Image2D> Radiance;
        Memory::Shared<Image2D> Irradiance;

        static Environment Create( const std::filesystem::path& filepath );
    };

    struct EnvironmentAttributes
    {
        float EnvironmentMapLod;
        float Intensity;
        float Rotation;
        float IBLContribution;
    };
} // namespace Radiant