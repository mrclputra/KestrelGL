cmake_minimum_required(VERSION 3.31)

project(KestrelGL)

# include_directories()

# use vcpkg, set libraries here
find_package(fmt CONFIG REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# add_executable(KestrelGL "src/main.cpp")
add_executable(KestrelGL main.cpp)

# link libraries
target_link_libraries(KestrelGL PRIVATE
  fmt::fmt
)

# compiler specific options here
if (MSVC)
    target_compile_options(KestrelGL PRIVATE /Zc:__cplusplus)
endif()