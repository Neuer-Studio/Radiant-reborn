cmake_minimum_required(VERSION 3.10) 
project(Radiant LANGUAGES CXX)

# Precompiled header
set(PRECOMPILED_HEADER "Source/rpch.hpp")

# Collect source files
file(GLOB_RECURSE ENGINE_SOURCES 
    "Source/Engine/*.cpp" 
    "Source/Engine/*.hpp"
)

set(SOURCES 
    # Precompiled header
    Source/rpch.cpp
    ${PRECOMPILED_HEADER}
    # Engine 
    ${ENGINE_SOURCES}
    # stb
    ../ThirdParty/stb/stb_image.cpp
)

if(SOURCES)
    add_library(Radiant STATIC ${SOURCES})
    target_precompile_headers(Radiant PRIVATE ${PRECOMPILED_HEADER})

    target_include_directories(Radiant PUBLIC
        Source/
        Include/
        Include/Radiant/
        ../ThirdParty/spdlog/include/
        ../ThirdParty/GLFW/include/
        ../ThirdParty/glm/
        ../ThirdParty/VulkanSDK/shaderc/Include
        ../ThirdParty/VulkanSDK/spirv_cross/Include
        ../ThirdParty/VulkanSDK/include
        ../ThirdParty/Glad/include/
        ../ThirdParty/stb/include/
        ../ThirdParty/assimp/include
        ../ThirdParty/yaml-cpp/include
        ../ThirdParty/ImGUI/
        ${IncludeDir_entt} 
        ../ThirdParty/
    )

    target_link_libraries(Radiant
        ImGui
        yaml-cpp
        ${LibraryDir_shaderc_Debug}
        ${LibraryDir_spirv_cross_core_Debug}
        ${LibraryDir_spirv_cross_glsl_Debug}
        ${LibraryDir_shaderc_shared_Debug}
        ${LibraryDir_shaderc_combined_Debug}
        ${LibraryDir_shaderc_util_Debug}
        ${LibraryDir_vulkan1}
        ${LibraryDir_assimp_Debug}
        ${LibraryDir_OGLCompiler_Debug}
    )

    target_compile_definitions(Radiant PRIVATE 
        YAML_CPP_STATIC_DEFINE
    )

    if(CMAKE_BUILD_TYPE MATCHES Debug)
        target_compile_definitions(Radiant PRIVATE RADIANT_CONFIG_DEBUG)
        target_compile_options(Radiant PRIVATE -g)
    else()
        target_compile_definitions(Radiant PRIVATE RADIANT_CONFIG_RELEASE)
    endif()

    if(WIN32)
        target_compile_definitions(Radiant PRIVATE RADIANT_PLATFORM_WINDOWS)
        
        file(GLOB_RECURSE WIN_FILES 
            "Source/Platform/Windows/*.cpp"
            "Source/Platform/Windows/*.hpp"
        )
        target_sources(Radiant PRIVATE ${WIN_FILES})
    endif()
else()
    message(FATAL_ERROR "No sources found for Radiant")
endif()
