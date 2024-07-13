local currentDir = _MAIN_SCRIPT_DIR
local dependenciesPath = currentDir .. "/BuildSystemDependencies/UNIX/MacOS/Dependencies.lua"

include(dependenciesPath)

project "Radiant"
    kind "StaticLib"

    pchheader "%{wks.location}/Radiant/Source/rpch.hpp"
    pchsource "%{wks.location}/Radiant/Source/rpch.cpp"
    forceincludes { "rpch.hpp" }

    files {
        "%{wks.location}/Radiant/Source/rpch.cpp", 
        "%{wks.location}/Radiant/Source/rpch.hpp", 
        
        "%{wks.location}/Radiant/Source/Engine/**.cpp", 
        "%{wks.location}/Radiant/Source/Engine/**.hpp", 
        "%{wks.location}/Radiant/Include/**.hpp",
        "%{wks.location}/ThirdParty/stb/stb_image.cpp"
    }

    files {
        "%{wks.location}/Radiant/Source/Platform/macOS/**.cpp", 
        "%{wks.location}/Radiant/Source/Platform/macOS/**.hpp"
    }

    libdirs {
        "/Users/daniilsavcenko/Desktop/Programming/C++/Radiant-reborn/build/Bin/Debug"
    }

    links{
        "libassimp"
    }

    removefiles
    {
        "%{wks.location}/Radiant/Source/Engine/Rendering/Platform/Vulkan**"
    }

    externalincludedirs {
        "%{wks.location}/Radiant/Source/", 
        "%{wks.location}/Radiant/Include/",
        "%{wks.location}/Radiant/Include/Radiant/",
        "%{wks.location}/ThirdParty/spdlog/include/",
        "%{wks.location}/ThirdParty/GLFW/include/",
        "%{wks.location}/ThirdParty/glm/",
        "%{wks.location}/ThirdParty/VulkanSDK/shaderc/Include",
        "%{wks.location}/ThirdParty/VulkanSDK/spirv_cross/Include", 
        "%{wks.location}/ThirdParty/VulkanSDK/include",
        "%{wks.location}/ThirdParty/Glad/include/", 
        "%{wks.location}/ThirdParty/stb/include/",
        "%{wks.location}/ThirdParty/assimp/include", 
        "%{wks.location}/ThirdParty/ImGUI/", 
        "%{IncludeDir.entt}",
        "%{wks.location}/ThirdParty/"
    }

    defines { "RADIANT_PLATFORM_MACOS" }
    links { "Cocoa.framework", "IOKit.framework", "CoreVideo.framework", "ImGui"}

    filter "configurations:Debug"
        defines { "RADIANT_CONFIG_DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "RADIANT_CONFIG_RELEASE" }
        optimize "On"
