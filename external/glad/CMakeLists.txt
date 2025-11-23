cmake_minimum_required(VERSION 3.31)

add_library(glad STATIC
  src/glad.c
)

target_include_directories(glad PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# link with OpenGL
# or any platforms that need it
if (WIN32)
  target_link_libraries(glad PUBLIC opengl32)
else()
  target_link_libraries(glad PUBLIC GL)
endif()