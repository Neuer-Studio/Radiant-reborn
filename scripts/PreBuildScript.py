import os

# Define the directory paths
target_dir = "../ThirdParty/yaml-cpp"
file_name = "premake5.lua"

# Ensure the target directory exists
os.makedirs(target_dir, exist_ok=True)

# Content of the premake5.lua file
file_content = """
project "yaml-cpp"
   kind "StaticLib"
   language "C++"
   
   files { "src/**.cpp", "include/**.h", "src/**.h", }

   includedirs { "include" }

   filter "system:windows"
      defines { "YAML_CPP_STATIC_DEFINE" }
      systemversion "latest"
      
   filter "system:not windows"
      pic "On"  -- Enable PIC for non-Windows platforms by default
"""

# Full path to the premake5.lua file
file_path = os.path.join(target_dir, file_name)

# Write the content to the file
with open(file_path, 'w') as file:
    file.write(file_content)

print(f"File {file_name} has been created successfully in {target_dir}.")
