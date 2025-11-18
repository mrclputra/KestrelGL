cmake_minimum_required(VERSION 3.31)

# Enable Hot Reload for MSVC compilers if supported.
# dont touch this block
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,\$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,\$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project(KestrelGL)

# include headers
include_directories(${PROJECT_SOURCE_DIR}/include)

# use vcpkg, set libraries here
find_package(fmt CONFIG REQUIRED)
find_package(glad REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(assimp REQUIRED)
find_package(imgui REQUIRED)

# set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# add_executable(KestrelGL "src/main.cpp")
add_executable(KestrelGL "src/main.cpp" "src/App.cpp")

# link libraries
target_link_libraries(KestrelGL PRIVATE
  fmt::fmt
  glm::glm
  glad::glad
  glfw
  assimp::assimp
  imgui::imgui
)

# custom include directories
target_include_directories(KestrelGL PRIVATE 
  ${PROJECT_SOURCE_DIR}/include
)

# compiler specific options here
if (MSVC)
    target_compile_options(KestrelGL PRIVATE /Zc:__cplusplus)
endif()

# TODO: add tests and install targets if needed.