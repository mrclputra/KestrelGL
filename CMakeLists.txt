cmake_minimum_required(VERSION 3.21)

project(KestrelGL
	VERSION 0.1.0
	LANGUAGES C CXX
)

# hot Reload for MSVC - don't touch this block
if(POLICY CMP0141)
    cmake_policy(SET CMP0141 NEW)
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# prevent in-source builds
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "In-source builds are not allowed. Please create a build directory.")
endif()

# assimp configuration
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)

# external dependencies
add_subdirectory(external/glad)
add_subdirectory(external/glfw)
add_subdirectory(external/imgui)
add_subdirectory(external/assimp)
add_subdirectory(external/ImGuiFileDialog)

# tell ImGuiFileDialog where imgui lives
target_include_directories(ImGuiFileDialog PUBLIC
    ${PROJECT_SOURCE_DIR}/external/imgui
)

# MAIN executable
add_executable(KestrelGL
    src/main.cpp
    src/App.cpp
    src/Scene.cpp
    src/Object.cpp
    src/Gui.cpp
    src/Camera.cpp    
    src/components/Texture.cpp
    src/components/Mesh.cpp
    src/debug.cpp
    src/ModelLoader.cpp
    src/Renderer.cpp
    src/Skybox.cpp
)

# target properties
target_compile_features(KestrelGL PRIVATE cxx_std_17)

# include directories
target_include_directories(KestrelGL PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/external/glm
)

# link libraries
target_link_libraries(KestrelGL PRIVATE
    glad
    glfw
    assimp
    imgui
    ImGuiFileDialog
)

if(MSVC)
    target_compile_options(KestrelGL PRIVATE /Zc:__cplusplus)
endif()

# TODO: add directory definitions here
target_compile_definitions(KestrelGL PRIVATE
    SHADER_DIR="${PROJECT_SOURCE_DIR}/src/shaders/"
)

# copy assets to build directory
add_custom_command(TARGET KestrelGL POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:KestrelGL>/assets"
    COMMENT "Copying assets to build directory..."
)