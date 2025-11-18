#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "App.hpp"

#include <iostream>

int main() {
  std::cout << "Hello Cmake" << std::endl;
  std::cout << "C++ Standard " << __cplusplus << std::endl;
  std::cout << "KestrelGL" << std::endl;

  App app(800, 600, "test");
  app.run();

  // initialize and configure GLFW

  // create window

  // load OpenGL GLAD function pointers
  
  // build and compile shader program

  // load models

  // setup scene

  // render loop

  // cleanup

  return 0;
}

