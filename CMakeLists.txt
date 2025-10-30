# CMakeList.txt : CMake project for KestrelGL, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.21)

# Enable Hot Reload for MSVC compilers if supported
# dont touch this block
if (POLICY CMP0141)
	cmake_policy(SET CMP0141 NEW)
	set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project (KestrelGL)

# use vcpkg, and define dependencies here
find_package(fmt CONFIG REQUIRED)
find_package(glad REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(imgui REQUIRED)
find_package(lodepng REQUIRED)

# set C++ standard here
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# copy shaders to build directory
file(COPY "${CMAKE_SOURCE_DIR}/assets" DESTINATION "${CMAKE_BINARY_DIR}")

# Add source to this project's executable.
# also link libraries
# add_executable (KestrelGL "KestrelGL.cpp" "KestrelGL.h")
add_executable(KestrelGL "src/main.cpp" "src/shader.h")

# link libraries here
target_link_libraries(KestrelGL PRIVATE fmt::fmt)
target_link_libraries(KestrelGL PRIVATE glad::glad)
target_link_libraries(KestrelGL PRIVATE glfw)

# compiler speficic options here
if (MSVC) 
	target_compile_options(KestrelGL PRIVATE /Zc:__cplusplus)
endif()

# if (CMAKE_VERSION VERSION_GREATER 3.12)
#	  set_property(TARGET KestrelGL PROPERTY CXX_STANDARD 20)
# endif()

# TODO: Add tests and install targets if needed.
# TODO: expand this file as project grows
