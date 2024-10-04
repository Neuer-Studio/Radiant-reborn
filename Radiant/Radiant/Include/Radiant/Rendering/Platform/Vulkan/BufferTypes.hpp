#pragma once

namespace Radiant::Rendering::Vulkan
{

    std::string GetBufferTypeString( const BufferTypes& type )
    {
        switch ( type )
        {
            case BufferTypes::VK_VertexBuffer:
                return "VK_VertexBuffer";
        }

        return "NONE";
    }

} // namespace Radiant::Rendering::Vulkan