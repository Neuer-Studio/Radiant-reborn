#include <vulkan/vulkan.hpp>

#include <Radiant/Rendering/Platform/Vulkan/VulkanBuffer.hpp>

namespace Radiant::Rendering::Vulkan
{

    namespace
    {
        Common::Result<VkBufferUsageFlags> GetVulkanBufferUsage( const BufferTypes& bufferType )
        {
            switch ( bufferType )
            {
                case BufferTypes::VK_VertexBuffer:
                    return Common::MakeSuccess<VkBufferUsageFlags>( VK_BUFFER_USAGE_VERTEX_BUFFER_BIT );
            }

            return Common::MakeError<VkBufferUsageFlags>(
                 "Can not found VkBufferUsageFlags( param: " + GetBufferTypeString( bufferType ) +
                 std::string( " )" ) );
        }
    } // namespace

    Common::Result<VkResult> VulkanBuffer::RT_CreateBuffer( const Radiant::VulkanDevice& device,
                                                            const BufferTypes& bufferType, uint32_t size )
    {
        VkBufferCreateInfo bufferCreateInfo;
        bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferCreateInfo.size  = size;

        const auto usage = GetVulkanBufferUsage( bufferType );

        if ( !usage.IsSuccess() )
        {
            return Common::MakeError<VkResult>( usage.GetError() );
        }

        bufferCreateInfo.usage = RA_GET_VALUE(usage.GetValue());
    }

} // namespace Radiant::Rendering::Vulkan