#pragma once 

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

//#include "../App.hpp"

class App;

class Gui {
public:
  Gui();
  ~Gui();

  void init(App* appPtr, GLFWwindow* window);
  void shutdown();

  void beginFrame();
  void endFrame();

  void draw();

private:
  App* app = nullptr;
  bool active = false; // flag
};