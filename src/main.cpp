#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <shader.hpp>
#include <model.hpp>
#include <mesh.hpp>
#include <camera.hpp>

#include "App.hpp"

#include <iostream>

int main() {
  std::cout << "Hello Cmake" << std::endl;
  std::cout << "C++ Standard " << __cplusplus << std::endl;
  std::cout << "KestrelGL" << std::endl;

  App app(1080, 720, "KestrelGL");
  app.run();

  return 0;
}

