cmake_minimum_required(VERSION 3.31)

# project name
project(KestrelGL)

# Enable Hot Reload for MSVC compilers if supported.
# dont touch this block
if (POLICY CMP0141)
	cmake_policy(SET CMP0141 NEW)
	set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,\$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,\$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# external dependencies
add_subdirectory(external/glad)
add_subdirectory(external/glfw)
add_subdirectory(external/imgui)
add_subdirectory(external/assimp)

add_subdirectory(external/ImGuiFileDialog)
target_include_directories(ImGuiFileDialog PUBLIC
	# tell ImGuiFileDialog where imgui.h is  
	${PROJECT_SOURCE_DIR}/external/imgui
)

# todo: lookup
# assimp options
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "" FORCE)

# copy assets to build directory
add_custom_target(copy_assets ALL
	COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/assets" "${CMAKE_CURRENT_BINARY_DIR}/assets"
	COMMENT "Copying assets..."
)

# executable
add_executable(KestrelGL 
	"src/main.cpp"
	"src/App.cpp" "src/App.h"
	"src/Scene.cpp" "src/Scene.h"
	"src/Entity.cpp" "src/Entity.h" 
	"src/Mesh.cpp" "src/Mesh.h" 
	"src/Gui.cpp" "src/Gui.h"
)

add_dependencies(KestrelGL copy_assets)

# include directories for targets
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

# compiler specific options here
if (MSVC)
	target_compile_options(KestrelGL PRIVATE /Zc:__cplusplus)
endif()

# TODO: add tests and install targets if needed.
target_compile_definitions(KestrelGL PRIVATE SHADER_DIR="${PROJECT_SOURCE_DIR}/src/shaders")