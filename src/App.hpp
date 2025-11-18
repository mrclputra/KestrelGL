#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <camera.hpp>

class App {
public:
  App(int width, int height, const char* title);
  ~App();

  void run();

  // callback handlers
  void onFrameBufferSize(int width, int height);
  void onCursorPos(double xPos, double yPos);
  void onScroll(double xOff, double yOff);
  void onKey(int key, int scancode, int action, int mods);

private:
  void init();
  void setupCallbacks();
  void cleanup();

  GLFWwindow* window;
  int width, height;
  const char* title;

  Camera camera;
  float lastX = 0.0f;
  float lastY = 0.0f;
  bool firstMouse = true;
};