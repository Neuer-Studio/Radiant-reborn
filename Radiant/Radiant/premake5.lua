
local currentDir = _MAIN_SCRIPT_DIR
local dependenciesPath = currentDir .. "/Dependencies.lua"
local dependencies = include(dependenciesPath)

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
        "%{wks.location}/ThirdParty/stb/stb_image.cpp",
    }

    includedirs {
        "Source/",
        "Include/",
        "%{wks.location}/Radiant/Common/Include/",
    }

    includedirs {
        "%{wks.location}/ThirdParty/spdlog/include/",
        "%{wks.location}/ThirdParty/GLFW/include/",
        "%{wks.location}/ThirdParty/glm/",
        "%{wks.location}/ThirdParty/VulkanSDK/shaderc/Include",
        "%{wks.location}/ThirdParty/VulkanSDK/spirv_cross/Include",
        "%{wks.location}/ThirdParty/VulkanSDK/include",
        "%{wks.location}/ThirdParty/Glad/include/",
        "%{wks.location}/ThirdParty/stb/include/",
        "%{wks.location}/ThirdParty/assimp/include",
        "%{wks.location}/ThirdParty/yaml-cpp/include",
      -- "../ThirdParty/boost/",
        --"../ThirdParty/boost/preprocessor",
        "%{wks.location}/ThirdParty/ImGUI/",
        "%{IncludeDir.entt}",
        
        "%{wks.location}/ThirdParty/",
    }

    local allLinks = {
        "ImGui",
        "Common",
    }

    for _, libPath in pairs(dependencies.LibraryDir) do
        table.insert(allLinks, libPath)
    end

    links(allLinks)

    defines { "YAML_CPP_STATIC_DEFINE" }
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