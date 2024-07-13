include "../Dependencies.lua"

project "Radiant"
    kind "StaticLib"

    pchheader "rpch.hpp"
    pchsource "Source/rpch.cpp"
    forceincludes { "rpch.hpp" }

    files { 
        -- Precompiled header
        "Source/rpch.cpp",
        "Source/rpch.hpp",
        
        -- Engine 
        "Source/Engine/**.cpp", 
        "Source/Engine/**.hpp",

        "Include/**.hpp",

        -- stb
        "../ThirdParty/stb/stb_image.cpp",
    }

    includedirs {
        "Source/",
        "Include/",
        "Include/Radiant/",
    }

    includedirs {
        "../ThirdParty/spdlog/include/",
        "../ThirdParty/GLFW/include/",
        "../ThirdParty/glm/",
        "../ThirdParty/VulkanSDK/shaderc/Include",
        "../ThirdParty/VulkanSDK/spirv_cross/Include",
        "../ThirdParty/VulkanSDK/include",
        "../ThirdParty/Glad/include/",
        "../ThirdParty/stb/include/",
        "../ThirdParty/assimp/include",
        "../ThirdParty/ImGUI/",
        "%{IncludeDir.entt}",
        
        "../ThirdParty/",
    }

    links
    {
        "ImGui",
        
        "%{LibraryDir.shaderc_Debug}",
        "%{LibraryDir.spirv_cross_core_Debug}",
        "%{LibraryDir.spirv_cross_glsl_Debug}",
        "%{LibraryDir.shaderc_shared_Debug}",
        "%{LibraryDir.shaderc_combined_Debug}",
        "%{LibraryDir.shaderc_util_Debug}",
        "%{LibraryDir.vulkan1}",
        "%{LibraryDir.assimp_Debug}",
        "%{LibraryDir.OGLCompiler_Debug}",
    }

    filter "configurations:Debug"
        defines { "RADIANT_CONFIG_DEBUG" }
        symbols "On"


    filter "configurations:Release"
        defines { "RADIANT_CONFIG_RELEASE" }

    filter { "system:windows" }
        defines { "RADIANT_PLATFORM_WINDOWS" }

        files {
            "Source/Platform/Windows/**.cpp",
            "Source/Platform/Windows/**.hpp",
        }